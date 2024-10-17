struct D3D12_State {
  static const u32 frame_count = 2;
  u32 frame_index;
  
  bool use_warp_device;
  ComPtr<ID3D12Device> device;
  ComPtr<ID3D12CommandQueue> command_queue;
  ComPtr<IDXGISwapChain3> swap_chain;
  
  ComPtr<ID3D12DescriptorHeap> rtv_heap;
  u32 rtv_descriptor_size;

  ComPtr<ID3D12Resource> render_targets[frame_count];
  ComPtr<ID3D12CommandAllocator> command_allocators[frame_count];

  ComPtr<ID3D12GraphicsCommandList> command_list;

  ComPtr<ID3D12RootSignature> root_signature;

  // Synchronization Objects
  HANDLE fence_event;
  ComPtr<ID3D12Fence> fence;
  u64 fence_values[frame_count];
  
  CD3DX12_VIEWPORT viewport;
  CD3DX12_RECT scissor;
};

struct D3D12_Shader {
  ComPtr<ID3DBlob> vertex;
  ComPtr<ID3DBlob> pixel;
};

struct D3D12_Pipeline {
    ComPtr<ID3D12PipelineState> state;
    D3D12_Shader *shader;
};

struct D3D12_Mesh {
  ComPtr<ID3D12Resource> vertex_buffer;
  D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
};
