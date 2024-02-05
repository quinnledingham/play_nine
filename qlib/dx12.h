struct DX12_Renderer {
	static const UINT back_buffer_count = 2;

	bool use_warp_device; // Adapter info

	// Initialization
	IDXGIFactory4* factory;
    IDXGIAdapter1* adapter;
#if defined(_DEBUG)
    ID3D12Debug1* debug_controller;
    ID3D12DebugDevice* debug_device;
#endif
    ID3D12Device* device;
    ID3D12CommandQueue* command_queue;
    ID3D12CommandAllocator* command_allocator;
    ID3D12GraphicsCommandList* command_list;

    // Current Frame
    UINT current_buffer;
    ID3D12DescriptorHeap* rtv_heap;
    ID3D12Resource* render_targets[back_buffer_count];
    IDXGISwapChain3* swap_chain;

	// App Resources
    D3D12_VIEWPORT viewport;
    D3D12_RECT scissor_rect;

	ID3D12Resource *vertex_buffer;
	ID3D12Resource *index_buffer;

	ID3D12Resource *unifrom_buffer;
	ID3D12DescriptorHeap* uniform_buffer_heap;
    UINT8 *mapped_uniform_buffer;

    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view;
    D3D12_INDEX_BUFFER_VIEW index_buffer_view;

    UINT rtv_descriptor_size;
    ID3D12RootSignature* root_signature;
    ID3D12PipelineState* pipeline_state;

	// Synchronization objects
	UINT frame_index;
	HANDLE fence_event;
	ID3D12Fence* fence;
	UINT64 fence_value;

    CD3DX12_RESOURCE_BARRIER barrier;
};

global DX12_Renderer dx12_renderer;