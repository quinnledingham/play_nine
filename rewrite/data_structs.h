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
      printf("WARNING: tried to access memory outside of Arr range. Returned last element instead\n");
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
      free(data);
    data = (T*)malloc(data_size);
  }

  ~Arr() {
    free(data);
  }
};

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
    data = (T*)malloc(data_size);
  }

  ~Stack() {
    free(data);
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
};