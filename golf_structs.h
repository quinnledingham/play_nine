template<typename T>
struct Stack {
  T *data = 0;
  u32 size;      // number of elements
  u32 type_size; // size of elements
  u32 data_size; // size * type_size = memory allocated

  u32 index;

  Stack(u32 in_size) {
    size = in_size;
    type_size = sizeof(T);
    data_size = type_size * size;
    data = (T*)SDL_malloc(data_size);
  }

  ~Stack() {
    SDL_free(data);
  }

  void push(T add) {
    data[index++] = add;
    ASSERT(index < size);
  }

  T pop() {
    T ret = data[index - 1];
    index--;
    return ret;
  }

  T top() {
    return data[index - 1];
  }

  u32 get_size() {
    return index;
  }

  bool8 empty() {
    if (index == 0)
      return true;
    else
      return false;
  }
};

struct Buffer {
  void *memory;
	u32 size;

	void clear() {
	  memset(memory, 0, size);
	}
};

internal Buffer
blank_buffer(u32 size) {
  Buffer result = {};
  result.size = size;
  result.memory = SDL_malloc(size);
  SDL_memset(result.memory, 0, size);
  return result;
}
