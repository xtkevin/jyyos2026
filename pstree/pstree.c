#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>

typedef struct proc_node{
    pid_t pid;
    pid_t ppid;
    char comm[256];
    struct proc_node **children;
    int n_children;
}proc_node;

proc_node **procs = NULL;
int n_process = 0;  //当前元素个数
int cap = 0;        //当前容量

int store_proc(proc_node *node){
    if(n_process == cap){
        cap = cap == 0 ? 16 : cap * 2;
        procs = realloc(procs, cap * sizeof(*procs));
        if(!procs){
            perror("realloc");
            exit(1);
        }
    }
    procs[n_process++] = node;

    return 0;
}


// 读取 /proc/<pid>/comm，获得进程的可执行文件名（一行字符串）
// 成功返回 0，失败返回 -1
static int read_comm(pid_t pid, char *buf, size_t n) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    // fgets 保留换行符，成功后再去掉行尾的 '\n'
    if (!fgets(buf, (int)n, f)) { fclose(f); return -1; }
    buf[strcspn(buf, "\n")] = 0;
    fclose(f);
    return 0;
}

// 读取 /proc/<pid>/stat，从中解析出父进程 PID (ppid)
// stat 文件格式示例："123 (bash) S 456 ..."，其中 456 就是 ppid
// 成功返回 0，失败返回 -1
static int get_ppid_from_stat(pid_t pid, pid_t *ppid_out) {
    char path[64], line[4096];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    if (!fgets(line, sizeof(line), f)) { fclose(f); return -1; }
    fclose(f);

    int id, ppid;
    char comm[256], state;
    // 用 sscanf 按固定格式解析前 4 个字段：pid, comm, state, ppid
    // %255[^)] 匹配括号内的 comm，避免 comm 本身含空格导致解析错乱
    if (sscanf(line, "%d (%255[^)]) %c %d", &id, comm, &state, &ppid) != 4) return -1;
    *ppid_out = (pid_t)ppid;
    return 0;
}

//读取系统所有的进程
int collect_procs(){
    // 遍历 /proc 目录，找出所有的进程
    DIR *d = opendir("/proc");
    if (!d) { perror("opendir /proc"); return 1; }
    struct dirent *de;
    while ((de = readdir(d)) != NULL) {
        // /proc 下以数字命名的目录对应各个进程
        if (!isdigit((unsigned char)de->d_name[0])) continue;
        pid_t pid = (pid_t)atoi(de->d_name);

        pid_t ppid;
        if(get_ppid_from_stat(pid, &ppid) != 0){
            continue;  // 进程可能刚好退出，跳过即可
        }

        char comm[256] = "?";
        read_comm(pid, comm, sizeof comm);
        proc_node* node = (proc_node*)malloc(sizeof(proc_node));
        node->pid = pid;
        strcpy(node->comm, comm);
        node->ppid = ppid;
        node->children = NULL;
        node->n_children = 0;
        store_proc(node);
    }
    closedir(d);
    return 0;
}


//构建进程树
int build_tree(){
    for(int i = 0; i < n_process; i++){
        int cnt = 0;
        for(int j = 0; j < n_process; j++){
            if(procs[i]->pid == procs[j]->ppid){
                cnt++;
            }
        }
        procs[i]->n_children = cnt;
        if(cnt == 0){
            procs[i]->children = NULL;
            continue;
        }
        procs[i]->children = malloc(cnt * sizeof(proc_node*));
        cnt = 0;
        for(int j = 0; j < n_process; j++){
            if(procs[i]->pid == procs[j]->ppid){
                procs[i]->children[cnt++] = procs[j];
            }
        }
    }
    return 0;
}

//递归打印进程树
int print_tree(proc_node *node, const char *prefix, bool is_last){
    // 打印当前节点
    if(prefix[0] == '\0'){
        // 根节点：没有连接线
        printf("%s(%d)", node->comm, node->pid);
    } else {
        // 非根节点：前缀 + "|- " 或 "`- "
        printf("%s%s%s(%d)", prefix, is_last ? "`- " : "|- ", node->comm, node->pid);
    }
    if(node->pid == getpid()){
        printf(" <== me");
    }
    printf("\n");

    // 构建给子节点使用的新前缀
    char child_prefix[4096];
    if(prefix[0] == '\0'){
        // 根节点的下一层统一缩进两个空格
        snprintf(child_prefix, sizeof(child_prefix), "  ");
    } else {
        // 根据当前节点是否是最后一个兄弟，决定后续用 "|   " 还是 "    "
        snprintf(child_prefix, sizeof(child_prefix), "%s%s", prefix, is_last ? "    " : "|   ");
    }

    // 递归打印每个子节点
    for(int i = 0; i < node->n_children; i++){
        print_tree(node->children[i], child_prefix, i == node->n_children - 1);
    }

    return 0;
}


int main(int argc, char *argv[]) {
    // 命令行参数解析：支持 -p/--show-pids, -n/--numeric-sort, -V/--version
    struct option long_options[] = {
        {"show-pids", no_argument, NULL, 'p'},
        {"numeric-sort", no_argument, NULL, 'n'},
        {"version", no_argument, NULL, 'V'},
        {0, 0, 0, 0}
    };
    int opt;
    bool show_pids = false;
    bool numeric_sort = false;
    bool show_version = false;
    while ((opt = getopt_long(argc, argv, "pnV", long_options, NULL)) != -1) {
        switch (opt) {
            case 'p':
                show_pids = true;
                break;
            case 'n':
                numeric_sort = true;
                break;
            case 'V':
                show_version = true;
                break;
            default:
                return 1;
        }
    }

    // --version 是独占选项，不能与其他选项混用
    if(show_version && (show_pids || numeric_sort)) {
        fprintf(stderr, "Error: --version cannot be combined with other options.\n");
        return 1;
    }
    if(show_version) {
        printf("pstree version 1.0\n");
        return 0;
    }

    // 收集所有进程并构建树
    if(collect_procs() != 0) return 1;
    if(build_tree() != 0) return 1;

    // 找到根节点：pid == 1（init/systemd）
    proc_node *root = NULL;
    for(int i = 0; i < n_process; i++){
        if(procs[i]->pid == 1){
            root = procs[i];
            break;
        }
    }
    if(!root){
        fprintf(stderr, "Error: cannot find init process (pid 1).\n");
        return 1;
    }

    // 从根节点开始递归打印
    print_tree(root, "", true);
    return 0;
}
