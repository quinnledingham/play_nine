// array where the size is bound to the data pointer
// only change the size and data pointer at the same time
template<typename T>
struct Arr {
private:
	T *data = 0;
	u32 size;  // number of elements of memory allocated at data

public:
	u32 data_size;
	u32 type_size;

	T& operator [] (int i) { 
		if (i >= (int)size) {
			print("WARNING: tried to access memory outside of Arr range. Returned last element instead\n");
			return data[size - 1];
		}

		return data[i]; 
	}

	T operator [] (int i) const {
		return (T)data[i];
	}

	T* get_data() {
		return data;
	}

	u32 get_size() const {
		return size;
	}

	void resize(u32 in_size) {
		size = in_size;
		type_size = sizeof(T);
		data_size = type_size * size;
		if (data != 0)
			platform_free(data);
		data = (T*)platform_malloc(data_size);
	}
};

struct LL_Node
{
    void *data;
    LL_Node *next;
    LL_Node *previous;
};

struct LL // Linked List
{
    LL_Node *head;
    u32 size;
    
    u32 data_size;
};

internal LL_Node*
create_ll_node(void *data) {
    LL_Node *node = (LL_Node*)platform_malloc(sizeof(LL_Node));
    *node = {};
    node->data = data;
    return node;
}

internal void
ll_add(LL *list, LL_Node *new_node) {
    if (list->size == 0)  {
        list->head = create_ll_node(0);
        list->head->next = new_node;
        new_node->previous = list->head;
    } else {
        LL_Node *node = list->head;
        for (u32 i = 0; i < list->size; i++) node = node->next;
        node->next = new_node;
        new_node->previous = node;
    }
    list->size++;
}

internal void
print_ll(LL* list) {
    LL_Node *node = list->head;
    for (u32 i = 0; i < list->size - 1; i++) { printf("%p\n", node->data); node = node->next; }
}

struct Lexer {
    File file;
    LL tokens;
    LL_Node *cursor;
    s32 line_num = 1;
    
    void *(*scan)(File *file, s32 *line_num);
    u32 token_size;
};

internal void*
lex(Lexer *lexer) {
    if (lexer->cursor == 0 || lexer->cursor->next == 0)  {
        void *token = lexer->scan(&lexer->file, &lexer->line_num);
        LL_Node *new_node = create_ll_node(token);
        ll_add(&lexer->tokens, new_node);
        lexer->cursor = new_node;
    } else {
        lexer->cursor = lexer->cursor->next;
    }
    
    return lexer->cursor->data;
}

internal void
unlex(Lexer *lexer) {
    lexer->cursor = lexer->cursor->previous;
}

internal void*
peek(Lexer *lexer) {
    void *token = lex(lexer);
    unlex(lexer);
    return token;
}

internal void
reset_lex(Lexer *lexer) {
    lexer->cursor = lexer->tokens.head;
}