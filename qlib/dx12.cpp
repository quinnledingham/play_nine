void dx12_get_hardware_adapter(_In_ IDXGIFactory1* p_factory, _Outptr_result_maybenull_ IDXGIAdapter1** pp_adapter, bool request_high_performance_adapter = false);

_Use_decl_annotations_
void dx12_get_hardware_adapter(IDXGIFactory1* p_factory, IDXGIAdapter1** pp_adapter, bool request_high_performance_adapter) {
    *pp_adapter = nullptr;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

    ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(p_factory->QueryInterface(IID_PPV_ARGS(&factory6)))) {
        for (UINT adapter_index = 0;
             SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                adapter_index,
                request_high_performance_adapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                IID_PPV_ARGS(&adapter)));
             ++adapter_index) {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
                break;
            }
        }
    }

    if(adapter.Get() == nullptr) {
        for (UINT adapter_index = 0; SUCCEEDED(p_factory->EnumAdapters1(adapter_index, &adapter)); ++adapter_index) {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
                break;
            }
        }
    }
    
    *pp_adapter = adapter.Detach();
}

void dx12_wait_for_previous_frame(DX12_Renderer *renderer, bool8 set_frame_index) {
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

	// Signal and increment the fence value.
	const UINT64 fence = renderer->fence_value;
	if (FAILED(renderer->command_queue->Signal(renderer->fence, fence))) 
		logprint("dx12_wait_for_previous_frame()", "Signal() failed\n");
	renderer->fence_value++;

	// Wait until the previous frame is finished.
	if (renderer->fence->GetCompletedValue() < fence) {
	    if (FAILED(renderer->fence->SetEventOnCompletion(fence, renderer->fence_event))) 
	    	logprint("dx12_wait_for_previous_frame()", "SetEventOnCompletion() failed\n");
	    WaitForSingleObject(renderer->fence_event, INFINITE);
	}

	if (set_frame_index)
		renderer->frame_index = renderer->swap_chain->GetCurrentBackBufferIndex();
}

internal void
dx12_setup_swap_chain(HWND window_handle, DX12_Renderer *renderer, Application_Window *window) {

    if (renderer->swap_chain != nullptr) {
        renderer->swap_chain->ResizeBuffers(renderer->back_buffer_count, window->width, window->height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    } else {
        DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
        swap_chain_desc.BufferCount = renderer->back_buffer_count;
        swap_chain_desc.BufferDesc.Width = window->width;
        swap_chain_desc.BufferDesc.Height = window->height;
        swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swap_chain_desc.OutputWindow = window_handle;
        swap_chain_desc.SampleDesc.Count = 1;
        swap_chain_desc.Windowed = TRUE;

    	IDXGISwapChain *swap_chain;

    	// Swap chain needs the queue so that it can force a flush on it.
        if (FAILED(renderer->factory->CreateSwapChain(renderer->command_queue, &swap_chain_desc, &swap_chain))) {
        	logprint("dx12_init_api()", "CreateSwapChain() failed\n");
        }

        renderer->swap_chain = (IDXGISwapChain3 *)swap_chain;
    }

    renderer->frame_index = renderer->swap_chain->GetCurrentBackBufferIndex();
}

internal void
dx12_init_frame_buffer(DX12_Renderer *renderer) {
	renderer->current_buffer = renderer->swap_chain->GetCurrentBackBufferIndex();

	// Create descriptor heaps
   	{
   		// Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
        rtv_heap_desc.NumDescriptors = renderer->back_buffer_count;
        rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        if (FAILED(renderer->device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&renderer->rtv_heap)))) {
        	logprint("dx12_init_frame_buffer()", "CreateDescriptorHeap() failed\n");
        }

        renderer->rtv_descriptor_size = renderer->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
   	}

   	// Create frame resources
   	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(renderer->rtv_heap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < renderer->back_buffer_count; n++) {
            if (FAILED(renderer->swap_chain->GetBuffer(n, IID_PPV_ARGS(&renderer->render_targets[n])))) {
            	logprint("dx12_init_frame_buffer()", "GetBuffer() failed\n");
            }
            renderer->device->CreateRenderTargetView(renderer->render_targets[n], nullptr, rtv_handle);
            rtv_handle.ptr += (1 * renderer->rtv_descriptor_size);
        }
   	}

}

internal void 
dx12_destroy_frame_buffer(DX12_Renderer *renderer) {
    for (size_t i = 0; i < renderer->back_buffer_count; ++i) {
        if (renderer->render_targets[i]) {
            renderer->render_targets[i]->Release();
            renderer->render_targets[i] = 0;
        }
    }

    if (renderer->rtv_heap) {
        renderer->rtv_heap->Release();
        renderer->rtv_heap = nullptr;
    }
}

internal void
dx12_resize_window(DX12_Renderer *renderer, Application_Window *window) {
	window->aspect_ratio = (float32)window->width / (float32)window->height;
    renderer->viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(window->width), static_cast<float>(window->height));
    renderer->scissor_rect = CD3DX12_RECT(0, 0, static_cast<LONG>(window->width), static_cast<LONG>(window->height));

	dx12_wait_for_previous_frame(renderer, false);

	dx12_destroy_frame_buffer(renderer);
	dx12_setup_swap_chain(GetActiveWindow(), renderer, window);
	dx12_init_frame_buffer(renderer);
}

internal void
dx12_init_api(DX12_Renderer *renderer) {
	// Create factory

	// Enable the D3D12 debug layer
	UINT dxgiFactoryFlags = 0;
#ifdef DEBUG
	ID3D12Debug *debug_controller;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)))) {
        debug_controller->EnableDebugLayer();

        // Enable additional debug layers.
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif // DEBUG

    if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&renderer->factory)))) {
    	logprint("dx12_init_api()", "CreateDXGIFactory2 failed\n");
    }

    // Create device
	if (renderer->use_warp_device) {
    	IDXGIAdapter *warp_adapter;
    	if (FAILED(renderer->factory->EnumWarpAdapter(IID_PPV_ARGS(&warp_adapter)))) {
    		logprint("dx12_init_api()", "EnumWarpAdapter() failed\n");
    	}

    	if (FAILED(D3D12CreateDevice(warp_adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&renderer->device)))) {
    		logprint("dx12_init_api()", "D3D12CreateDevice() warp adapter failed\n");
    	}

    } else {
    	IDXGIAdapter1 *hardware_adapter;
    	dx12_get_hardware_adapter(renderer->factory, &hardware_adapter);

    	if (FAILED(D3D12CreateDevice(hardware_adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&renderer->device)))) {
    		logprint("dx12_init_api()", "D3D12CreateDevice() hardware adapter failed\n");
    	}
    }

    // Create Command Queue
    D3D12_COMMAND_QUEUE_DESC queue_desc = {};
    queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    if (FAILED(renderer->device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&renderer->command_queue)))) {
    	logprint("dx12_init_api()", "CreateCommandQueue() failed\n");
    }

    // Create Command Allocator
	if (FAILED(renderer->device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&renderer->command_allocator)))) {
		logprint("dx12_init_api()", "CreateCommandAllocator() failed\n");
	}

	// Sync
    if (FAILED(renderer->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&renderer->fence)))) {
    	logprint("dx12_init_api()", "CreateFence() failed\n");
    } 
}

internal void
dx12_init_resources(DX12_Renderer *renderer, Application_Window *window) {
	// Create an empty root signature.
    {
        CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc;
        root_signature_desc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        if (FAILED(D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error))) 
        	logprint("dx12_init_resources()", "D3D12SerializeRootSignature() failed\n");
        if (FAILED(renderer->device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&renderer->root_signature)))) 
        	logprint("dx12_init_resources()", "CreateRootSignature() failed\n");
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        ComPtr<ID3DBlob> vertex_shader;
        ComPtr<ID3DBlob> pixel_shader;

#ifdef DEBUG
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif
        //const char *file = (const char *)read_file_terminated("../shaders.hlsl").memory;
        LPCWSTR file = L"../dx12/basic_shaders.hlsl";

        ID3DBlob *error_messages;
        if (FAILED(D3DCompileFromFile(file, nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertex_shader, &error_messages))) {
        	logprint("dx12_init_resources()", "D3DCompileFromFile() VSMain failed\n");
            print("%s\n", (const char*)error_messages->GetBufferPointer());
        }
        if (FAILED(D3DCompileFromFile(file, nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixel_shader, &error_messages))) { 
        	logprint("dx12_init_resources()", "D3DCompileFromFile() PSMain failed\n");
            print("%s\n", (const char*)error_messages->GetBufferPointer());
        }

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC input_element_descs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
        pso_desc.InputLayout = { input_element_descs, _countof(input_element_descs) };
        pso_desc.pRootSignature = renderer->root_signature;
        pso_desc.VS = { reinterpret_cast<UINT8*>(vertex_shader->GetBufferPointer()), vertex_shader->GetBufferSize() };
        pso_desc.PS = { reinterpret_cast<UINT8*>(pixel_shader->GetBufferPointer()), pixel_shader->GetBufferSize() };
        pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        pso_desc.DepthStencilState.DepthEnable = FALSE;
        pso_desc.DepthStencilState.StencilEnable = FALSE;
        pso_desc.SampleMask = UINT_MAX;
        pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso_desc.NumRenderTargets = 1;
        pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso_desc.SampleDesc.Count = 1;
        if (FAILED(renderer->device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&renderer->pipeline_state)))) 
        	logprint("dx12_init_resources()", "CreateRootSignature() failed\n");
    }

    // Create the command list.
	if (FAILED(renderer->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, renderer->command_allocator, renderer->pipeline_state, IID_PPV_ARGS(&renderer->command_list))))
		logprint("dx12_init_resources()", "CreateRootSignature() failed\n");

	// Command lists are created in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
	if (FAILED(renderer->command_list->Close()))
		logprint("dx12_init_resources()", "CreateRootSignature() failed\n");
  

    // Create the vertex buffer.
    /*
    {
        // Define the geometry for a triangle.
        Vertex_XNU triangle_vertices[] =
        {
            { {   0.0f,  0.25f * window->aspect_ratio, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.5f, 1.0f } },
            { {  0.25f, -0.25f * window->aspect_ratio, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
            { { -0.25f, -0.25f * window->aspect_ratio, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } }
        };

        const UINT vertex_buffer_size = sizeof(triangle_vertices);

        // Note: using upload heaps to transfer static data like vert buffers is not 
        // recommended. Every time the GPU needs it, the upload heap will be marshalled 
        // over. Please read up on Default Heap usage. An upload heap is used here for 
        // code simplicity and because there are very few verts to actually transfer.
        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
        auto desc = CD3DX12_RESOURCE_DESC::Buffer(vertex_buffer_size);
        HRESULT result = renderer->device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&renderer->vertex_buffer));
        if (FAILED(result)) 
        	logprint("dx12_init_resources()", "CreateRootSignature() failed\n");

        // Copy the triangle data to the vertex buffer.
        UINT8* p_vertex_data_begin;
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        result = renderer->vertex_buffer->Map(0, &readRange, reinterpret_cast<void**>(&p_vertex_data_begin));
        if (FAILED(result)) 
        	logprint("dx12_init_resources()", "CreateRootSignature() failed\n");
        memcpy(p_vertex_data_begin, triangle_vertices, sizeof(triangle_vertices));
        renderer->vertex_buffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        renderer->vertex_buffer_view.BufferLocation = renderer->vertex_buffer->GetGPUVirtualAddress();
        renderer->vertex_buffer_view.StrideInBytes = sizeof(Vertex_XNU);
        renderer->vertex_buffer_view.SizeInBytes = vertex_buffer_size;
    }
    */

	// Create synchronization objects and wait until assets have been uploaded to the GPU
    /*
	{
		renderer->fence_value = 1;

		// Create an event handle to use for frame synchornization
		renderer->fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (renderer->fence_event == nullptr) {
            if (FAILED(HRESULT_FROM_WIN32(GetLastError()))) 
            	logprint("dx12_init_resources()", "CreateEvent() failed\n");
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        dx12_wait_for_previous_frame(renderer, true);
	}
    */
}

void init_gpu_mesh(Triangle_Mesh *mesh) {
    DX12_Renderer *renderer = &dx12_renderer;

    // Vertex Buffer
    {
        const u32 vertex_buffer_size = mesh->vertices_count * sizeof(mesh->vertices[0]);

        // Note: using upload heaps to transfer static data like vert buffers is not 
        // recommended. Every time the GPU needs it, the upload heap will be marshalled 
        // over. Please read up on Default Heap usage. An upload heap is used here for 
        // code simplicity and because there are very few verts to actually transfer.
        CD3DX12_HEAP_PROPERTIES heap_props(D3D12_HEAP_TYPE_UPLOAD);
        auto desc = CD3DX12_RESOURCE_DESC::Buffer(vertex_buffer_size);
        if (FAILED(renderer->device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&renderer->vertex_buffer)))) 
            logprint("dx12_init_resources()", "CreateCommittedResource() failed\n");

        // Copy the triangle data to the vertex buffer.
        UINT8* p_vertex_data_begin;
        CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        if (FAILED(renderer->vertex_buffer->Map(0, &readRange, reinterpret_cast<void**>(&p_vertex_data_begin)))) 
            logprint("dx12_init_resources()", "Map() failed\n");
        memcpy(p_vertex_data_begin, mesh->vertices, vertex_buffer_size);
        renderer->vertex_buffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        renderer->vertex_buffer_view.BufferLocation = renderer->vertex_buffer->GetGPUVirtualAddress();
        renderer->vertex_buffer_view.StrideInBytes = sizeof(Vertex_XNU);
        renderer->vertex_buffer_view.SizeInBytes = vertex_buffer_size;

        mesh->vertex_buffer_object = renderer->vertex_buffer->GetGPUVirtualAddress();
    }

    // Index Buffer
    {
        const u32 index_buffer_size = mesh->indices_count * sizeof(mesh->indices[0]);

        // Note: using upload heaps to transfer static data like vert buffers is not 
        // recommended. Every time the GPU needs it, the upload heap will be marshalled 
        // over. Please read up on Default Heap usage. An upload heap is used here for 
        // code simplicity and because there are very few verts to actually transfer.
        CD3DX12_HEAP_PROPERTIES heap_props(D3D12_HEAP_TYPE_UPLOAD);
        auto desc = CD3DX12_RESOURCE_DESC::Buffer(index_buffer_size);
        if (FAILED(renderer->device->CreateCommittedResource(&heap_props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&renderer->index_buffer)))) 
            logprint("dx12_init_resources()", "CreateCommittedResource() failed\n");

        // Copy the triangle data to the index buffer.
        UINT8* p_index_data_begin;
        CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        if (FAILED(renderer->index_buffer->Map(0, &readRange, reinterpret_cast<void**>(&p_index_data_begin)))) 
            logprint("dx12_init_resources()", "Map() failed\n");
        memcpy(p_index_data_begin, mesh->indices, index_buffer_size);
        renderer->index_buffer->Unmap(0, nullptr);

        // Initialize the index buffer view.
        renderer->index_buffer_view.BufferLocation = renderer->index_buffer->GetGPUVirtualAddress();
        renderer->index_buffer_view.Format = DXGI_FORMAT_R32_UINT;
        renderer->index_buffer_view.SizeInBytes = index_buffer_size;

        mesh->index_buffer_object = renderer->index_buffer->GetGPUVirtualAddress();
    }

    {
        renderer->fence_value = 1;

        // Create an event handle to use for frame synchornization
        renderer->fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (renderer->fence_event == nullptr) {
            if (FAILED(HRESULT_FROM_WIN32(GetLastError()))) 
                logprint("dx12_init_resources()", "CreateEvent() failed\n");
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        dx12_wait_for_previous_frame(renderer, true);
    }
}

internal void
draw_triangle_mesh(Triangle_Mesh *mesh) {
    const u32 vertex_buffer_size = mesh->vertices_count * sizeof(mesh->vertices[0]);
    const u32 index_buffer_size = mesh->indices_count * sizeof(mesh->indices[0]);

    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view = {};
    vertex_buffer_view.BufferLocation = mesh->vertex_buffer_object;
    vertex_buffer_view.StrideInBytes = sizeof(Vertex_XNU);
    vertex_buffer_view.SizeInBytes = vertex_buffer_size;

    D3D12_INDEX_BUFFER_VIEW index_buffer_view = {};
    index_buffer_view.BufferLocation = mesh->index_buffer_object;
    index_buffer_view.Format = DXGI_FORMAT_R32_UINT;
    index_buffer_view.SizeInBytes = index_buffer_size;

    DX12_Renderer *renderer = &dx12_renderer;
    renderer->command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    renderer->command_list->IASetVertexBuffers(0, 1, &vertex_buffer_view);
    renderer->command_list->IASetIndexBuffer(&index_buffer_view);
    renderer->command_list->DrawIndexedInstanced(mesh->indices_count, 1, 0, 0, 0);
}

internal void
dx12_start_commands(DX12_Renderer *renderer) {
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    if (FAILED(renderer->command_allocator->Reset()))
        logprint("dx12_setup_commands()", "command_allocator Reset() failed\n");

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    if (FAILED(renderer->command_list->Reset(renderer->command_allocator, renderer->pipeline_state))) {
        logprint("dx12_setup_commands()", "command_list Reset() failed\n");
    }

    // Set necessary state.
    renderer->command_list->SetGraphicsRootSignature(renderer->root_signature);
    renderer->command_list->RSSetViewports(1, &renderer->viewport);
    renderer->command_list->RSSetScissorRects(1, &renderer->scissor_rect);

    // Indicate that the back buffer will be used as a render target.
    renderer->barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderer->render_targets[renderer->frame_index], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    renderer->command_list->ResourceBarrier(1, &renderer->barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(renderer->rtv_heap->GetCPUDescriptorHandleForHeapStart(), renderer->frame_index, renderer->rtv_descriptor_size);
    renderer->command_list->OMSetRenderTargets(1, &rtv_handle, FALSE, nullptr);

    // Record commands.
    renderer->command_list->ClearRenderTargetView(rtv_handle, renderer_clear_color.E, 0, nullptr);
}

internal void 
dx12_end_commands(DX12_Renderer *renderer) {
    // Indicate that the back buffer will now be used to present.
    renderer->barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderer->render_targets[renderer->frame_index], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    renderer->command_list->ResourceBarrier(1, &renderer->barrier);

    if (FAILED(renderer->command_list->Close())) {
		logprint("dx12_setup_commands()", "command_list->Close() failed\n");
    }
}

internal void
dx12_render(DX12_Renderer *renderer) {
	dx12_end_commands(renderer);

	// Execute the command list.
    ID3D12CommandList* pp_command_lists[] = { renderer->command_list };
    renderer->command_queue->ExecuteCommandLists(_countof(pp_command_lists), pp_command_lists);

	// Present the frame.
    if (FAILED(renderer->swap_chain->Present(1, 0))) 
    	logprint("dx12_render()", "Present() failed\n");

    dx12_wait_for_previous_frame(renderer, true);
}

internal void
dx12_destory() {
	// Wait for the GPU to be done with all resources.
    //dx_wait_for_previous_frame(input);

    //CloseHandle(renderer->m_fence_event);
}

internal void
dx12_init(DX12_Renderer *renderer, Application_Window *window) {
	renderer->frame_index = 0;
	renderer->rtv_descriptor_size = 0;

	dx12_init_api(renderer);
	dx12_resize_window(renderer, window);
	dx12_init_resources(renderer, window);
	//dx12_setup_commands(renderer);
}
