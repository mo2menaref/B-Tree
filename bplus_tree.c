/*
 * B+ Tree Implementation with File Persistence
 * Features: Insert, Display, Delete (Clear All)
 * Persistent storage between requests
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ORDER 3
#define MAX_KEYS (ORDER - 1)
#define DATA_FILE "btree.dat"

// Node structure for file storage
typedef struct {
    int keys[MAX_KEYS + 1];
    int children[ORDER + 1];  // File positions
    int parent;
    int next;
    int num_keys;
    int is_leaf;
} NodeData;

// Tree metadata
typedef struct {
    int root_pos;
    int next_pos;
    int node_count;
} TreeMeta;

FILE *fp = NULL;

// Write metadata
void write_meta(TreeMeta *meta) {
    fseek(fp, 0, SEEK_SET);
    fwrite(meta, sizeof(TreeMeta), 1, fp);
    fflush(fp);
}

// Read metadata
TreeMeta read_meta() {
    TreeMeta meta;
    fseek(fp, 0, SEEK_SET);
    fread(&meta, sizeof(TreeMeta), 1, fp);
    return meta;
}

// Initialize file
void init_file() {
    fp = fopen(DATA_FILE, "rb+");
    
    if (fp == NULL) {
        // Create new file
        fp = fopen(DATA_FILE, "wb+");
        
        TreeMeta meta;
        meta.root_pos = sizeof(TreeMeta);
        meta.next_pos = sizeof(TreeMeta) + sizeof(NodeData);
        meta.node_count = 1;
        
        write_meta(&meta);
        
        // Create empty root
        NodeData root;
        memset(&root, 0, sizeof(NodeData));
        root.is_leaf = 1;
        root.parent = -1;
        root.next = -1;
        for (int i = 0; i <= ORDER; i++) {
            root.children[i] = -1;
        }
        
        fseek(fp, meta.root_pos, SEEK_SET);
        fwrite(&root, sizeof(NodeData), 1, fp);
        fflush(fp);
    }
}

// Read node
NodeData read_node(int pos) {
    NodeData node;
    fseek(fp, pos, SEEK_SET);
    fread(&node, sizeof(NodeData), 1, fp);
    return node;
}

// Write node
void write_node(int pos, NodeData *node) {
    fseek(fp, pos, SEEK_SET);
    fwrite(node, sizeof(NodeData), 1, fp);
    fflush(fp);
}

// Allocate new node
int alloc_node(bool is_leaf) {
    TreeMeta meta = read_meta();
    int pos = meta.next_pos;
    meta.next_pos += sizeof(NodeData);
    meta.node_count++;
    write_meta(&meta);
    
    NodeData node;
    memset(&node, 0, sizeof(NodeData));
    node.is_leaf = is_leaf;
    node.parent = -1;
    node.next = -1;
    for (int i = 0; i <= ORDER; i++) {
        node.children[i] = -1;
    }
    
    write_node(pos, &node);
    return pos;
}

// Find leaf for key
int find_leaf(int pos, int key) {
    NodeData node = read_node(pos);
    
    if (node.is_leaf) {
        return pos;
    }
    
    int i = 0;
    while (i < node.num_keys && key >= node.keys[i]) {
        i++;
    }
    
    return find_leaf(node.children[i], key);
}

// Split node
void split_node(int node_pos) {
    NodeData node = read_node(node_pos);
    int new_pos = alloc_node(node.is_leaf);
    NodeData new_node = read_node(new_pos);
    
    int mid = node.num_keys / 2;
    int push_key;
    
    if (node.is_leaf) {
        // Leaf split
        for (int i = mid; i < node.num_keys; i++) {
            new_node.keys[i - mid] = node.keys[i];
        }
        new_node.num_keys = node.num_keys - mid;
        node.num_keys = mid;
        
        new_node.next = node.next;
        node.next = new_pos;
        push_key = new_node.keys[0];
    } else {
        // Internal split
        push_key = node.keys[mid];
        
        for (int i = mid + 1; i < node.num_keys; i++) {
            new_node.keys[i - mid - 1] = node.keys[i];
        }
        new_node.num_keys = node.num_keys - mid - 1;
        
        for (int i = mid + 1; i <= node.num_keys; i++) {
            new_node.children[i - mid - 1] = node.children[i];
            if (node.children[i] != -1) {
                NodeData child = read_node(node.children[i]);
                child.parent = new_pos;
                write_node(node.children[i], &child);
            }
            node.children[i] = -1;
        }
        
        node.num_keys = mid;
    }
    
    write_node(node_pos, &node);
    write_node(new_pos, &new_node);
    
    // Insert into parent
    if (node.parent == -1) {
        // Create new root
        int new_root = alloc_node(false);
        NodeData root;
        memset(&root, 0, sizeof(NodeData));
        root.is_leaf = 0;
        root.parent = -1;
        root.next = -1;
        root.keys[0] = push_key;
        root.children[0] = node_pos;
        root.children[1] = new_pos;
        root.num_keys = 1;
        
        for (int i = 2; i <= ORDER; i++) {
            root.children[i] = -1;
        }
        
        write_node(new_root, &root);
        
        node.parent = new_root;
        new_node.parent = new_root;
        write_node(node_pos, &node);
        write_node(new_pos, &new_node);
        
        TreeMeta meta = read_meta();
        meta.root_pos = new_root;
        write_meta(&meta);
    } else {
        // Insert into existing parent
        int parent_pos = node.parent;
        NodeData parent = read_node(parent_pos);
        new_node.parent = parent_pos;
        write_node(new_pos, &new_node);
        
        int i = parent.num_keys - 1;
        while (i >= 0 && push_key < parent.keys[i]) {
            parent.keys[i + 1] = parent.keys[i];
            parent.children[i + 2] = parent.children[i + 1];
            i--;
        }
        
        parent.keys[i + 1] = push_key;
        parent.children[i + 2] = new_pos;
        parent.num_keys++;
        
        write_node(parent_pos, &parent);
        
        if (parent.num_keys > MAX_KEYS) {
            split_node(parent_pos);
        }
    }
}

// Insert key
void insert(int key) {
    TreeMeta meta = read_meta();
    int leaf_pos = find_leaf(meta.root_pos, key);
    NodeData leaf = read_node(leaf_pos);
    
    // Check duplicate
    for (int i = 0; i < leaf.num_keys; i++) {
        if (leaf.keys[i] == key) {
            printf("Key %d already exists!\n", key);
            return;
        }
    }
    
    // Insert in sorted order
    int i = leaf.num_keys - 1;
    while (i >= 0 && key < leaf.keys[i]) {
        leaf.keys[i + 1] = leaf.keys[i];
        i--;
    }
    leaf.keys[i + 1] = key;
    leaf.num_keys++;
    
    write_node(leaf_pos, &leaf);
    
    if (leaf.num_keys > MAX_KEYS) {
        split_node(leaf_pos);
    }
    
    printf("Inserted %d successfully!\n", key);
}

// Display helper
void display_level(int pos, int level, int target, int *count) {
    if (pos == -1) return;
    
    NodeData node = read_node(pos);
    
    if (level == target) {
        (*count)++;
        printf("[");
        for (int i = 0; i < node.num_keys; i++) {
            printf("%d", node.keys[i]);
            if (i < node.num_keys - 1) printf(",");
        }
        printf("]");
        if (node.is_leaf) printf("(L) ");
        else printf("(I) ");
        return;
    }
    
    if (!node.is_leaf) {
        for (int i = 0; i <= node.num_keys; i++) {
            display_level(node.children[i], level + 1, target, count);
        }
    }
}

// Get height
int get_height(int pos) {
    if (pos == -1) return 0;
    NodeData node = read_node(pos);
    if (node.is_leaf) return 1;
    return 1 + get_height(node.children[0]);
}

// Display tree
void display_tree() {
    printf("\n========================================\n");
    printf("         B+ TREE (Order %d)\n", ORDER);
    printf("========================================\n\n");
    
    TreeMeta meta = read_meta();
    NodeData root = read_node(meta.root_pos);
    
    if (root.num_keys == 0) {
        printf("Tree is empty.\n");
        printf("\n========================================\n");
        return;
    }
    
    int height = get_height(meta.root_pos);
    
    for (int i = 0; i < height; i++) {
        printf("Level %d: ", i);
        int count = 0;
        display_level(meta.root_pos, 0, i, &count);
        printf("\n");
    }
    
    printf("\n========================================\n");
    printf("Total nodes: %d\n", meta.node_count);
    printf("Tree height: %d\n", height);
    printf("Max keys per node: %d\n", MAX_KEYS);
    printf("========================================\n");
}

// Clear all
void clear_all() {
    if (fp) {
        fclose(fp);
        fp = NULL;
    }
    remove(DATA_FILE);
    init_file();
    printf("Tree cleared successfully!\n");
}

// Main
int main() {
    printf("Content-Type: text/plain\r\n\r\n");
    
    char *method = getenv("REQUEST_METHOD");
    char operation[50] = "display";
    int value = 0;
    
    if (method && strcmp(method, "POST") == 0) {
        char *len_str = getenv("CONTENT_LENGTH");
        if (len_str) {
            int len = atoi(len_str);
            if (len > 0 && len < 10000) {
                char *data = malloc(len + 1);
                fread(data, 1, len, stdin);
                data[len] = '\0';
                
                char *op = strstr(data, "operation=");
                if (op) {
                    op += 10;
                    int i = 0;
                    while (op[i] && op[i] != '&' && i < 49) {
                        operation[i] = op[i];
                        i++;
                    }
                    operation[i] = '\0';
                }
                
                char *val = strstr(data, "value=");
                if (val) {
                    val += 6;
                    value = atoi(val);
                }
                
                free(data);
            }
        }
    }
    
    init_file();
    
    if (strcmp(operation, "insert") == 0) {
        printf("=== INSERT %d ===\n\n", value);
        insert(value);
        printf("\n");
        display_tree();
    } else if (strcmp(operation, "clear") == 0) {
        printf("=== CLEAR ALL ===\n\n");
        clear_all();
        printf("\n");
        display_tree();
    } else {
        display_tree();
    }
    
    if (fp) fclose(fp);
    return 0;
}