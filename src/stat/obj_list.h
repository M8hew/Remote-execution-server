enum {
    CREATED = 0,
    ENDED = 1,
};

typedef struct Node {
    struct Node *next;

    long long time;
    int type;
} Node;

void init_list();

void add_node(long long time, int type);

Node *get_tail();

void delete_tail();