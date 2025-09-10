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
struct Array {
  T *data;

  // data_size = type_size * element_count
  u32 data_size;
  u32 type_size;
  u32 element_count;

  u32 insert_index; // the amount of elements that have been added

  Array() {
    data = 0;
    insert_index = 0;
    element_count = 0;
  }

  void init(u32 in_element_count) {
    element_count = in_element_count;
    type_size = sizeof(T);
    data_size = type_size * element_count;
    data = (T*)malloc(data_size);
  }

  void clear() {
    memset(data, 0, data_size);
    insert_index = 0;
  }

  u32 size() {
    return element_count;
  }

  void insert(T new_element) {
    //ASSERT(insert_index < element_count);
    if (insert_index >= element_count) {
      if (!data) {
        init(1);
      } else {
        T *old_data = data;
        u32 old_element_count = element_count;
        init(element_count * 2);
        memcpy(data, old_data, type_size * old_element_count);
        free(old_data);
      }
    }
    data[insert_index++] = new_element;
  }
  
  T& top() {
    return data[insert_index - 1];
  }

  T& operator [] (int i) { 
#ifdef DEBUG
    if (i < 0) {
      log_error("WARNING: tried to access memory outside of Arr range. Returned first element instead\n");
      return data[0];
    } else if (i >= (int)element_count) {
      log_error("WARNING: tried to access memory outside of Arr range. Returned last element instead\n");
      return data[element_count - 1];
    }
#endif // DEBUG

    return data[i]; 
  }

  void destroy() {
    free(data);
  }
};

//
// controls how data is added to the array so that indices can be reused
//
template<typename T>
struct ArrayB_Element {
  bool8 in_use;
  T data;
};  

template<typename T>
struct ArrayB {
private:
  ArrayB_Element<T> *data;
  
  u32 data_size;
  u32 type_size;
  u32 max_elements; // how many of type can fit in data_size

public:
  ArrayB() {
    data = 0;
    max_elements = 0;
  }

  void init(u32 in_max_elements) {
    max_elements = in_max_elements;
    type_size = sizeof(T);
    data_size = sizeof(ArrayB_Element<T>) * max_elements;
    data = (ArrayB_Element<T>*)malloc(data_size);
    memset(data, 0, data_size);
  }

  T* open() {
    for (u32 i = 0; i < max_elements; i++) {
      ArrayB_Element<T> *element = &data[i];
      if (!element->in_use) {
        element->in_use = true;
        element->data = {};
        return &element->data;
      }
    }
    ASSERT(0);
    return 0;
  }

  void close(T* ptr) {
    for (u32 i = 0; i < max_elements; i++) {
      ArrayB_Element<T> *element = &data[i];
      if (&element->data == ptr) {
        element->in_use = false;
        return;
      }
    }
    ASSERT(0);
  }

  u32 size() {
    return max_elements;
  }

  T* get(u32 i) {
    ArrayB_Element<T> *element = &data[i];
    if (!element->in_use)
      return 0;
    return &element->data;
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

// centers in inside of out
inline Vector2_s32
centered(Vector2_s32 out, Vector2_s32 in) {
  return {
    (out.x / 2) - (in.x / 2),
    (out.y / 2) - (in.y / 2),
  };
}