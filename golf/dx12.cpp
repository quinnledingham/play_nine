internal void
dx_get_hardware_adapter(IDXGIFactory1 *p_factory, IDXGIAdapter1 **pp_adapter, bool request_high_performance_adapter) {
  *pp_adapter = nullptr;
  ComPtr<IDXGIAdapter1> adapter;
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
        // Don't select the basic render driver adapter.
        // If you want a software adapter, pass in "/warp" on the command line.
        continue;
      }

      // Check to see whether the adapter supports Direct3D 12, but don't create the actual device yet.
      if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
        break;
      }
    }
  }

  if (adapter.Get() == nullptr) {
    for (UINT adapter_index = 0;
         SUCCEEDED(p_factory->EnumAdapters1(adapter_index, &adapter));
         ++adapter_index) {
      DXGI_ADAPTER_DESC1 desc;
      adapter->GetDesc1(&desc);
      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        // Don't select the basic render driver adapter.
        // If you want a software adapter, pass in "/warp" on the command line.
        continue;
      }
      
      // Check to see whether the adapter supports Direct3D 12, but don't create the actual device yet.
      if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
        break;
      }
    }
  }

  *pp_adapter = adapter.Detach();
}

internal void
dx_init(DX_State *state, HWND window_handle, Vector2_s32 window_dim) {
  state->frame_index = 0;
  state->rtv_descriptor_size = 0;
  
  // Enable the D3D12 debug layer
  UINT dxgi_factory_flags = 0;
#ifdef GOLF_DEBUG

  ID3D12Debug *debug_controller;
  if (SUCCEEDED(D3D12GetDebugInterface(&debug_controller))) {
    debug_controller->EnableDebugLayer();
  }

  dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;

#endif // GOLF_DEBUG

  // Create the factory
  ComPtr<IDXGIFactory4> factory;
  if (FAILED(CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&factory)))) {
      win32_print_str("dx_init(): CreateDXGIFactory2() failed\n");
  }

  if (state->use_warp_device) {
    ComPtr<IDXGIAdapter> warp_adapter;
    if (FAILED(factory->EnumWarpAdapter(IID_PPV_ARGS(&warp_adapter)))) {
      win32_print_str("dx_init(): EnumWarpAdapter() failed");
    }

    if (FAILED(D3D12CreateDevice(warp_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&state->device)))) {
      win32_print_str("dx_init(): D3D12CreateDevice() warp adapter failed");
    }
  } else {
    ComPtr<IDXGIAdapter1> hardware_adapter;
    dx_get_hardware_adapter(factory.Get(), &hardware_adapter, true);

    if (FAILED(D3D12CreateDevice(hardware_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&state->device)))) {
      win32_print_str("dx_init(): D3D12CreateDevice() hardware adapter failed");
    }
  }

  // Create the command queue
  D3D12_COMMAND_QUEUE_DESC queue_desc = {};
  queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  state->device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&state->command_queue));

  // Create swap chain
  DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
  swap_chain_desc.BufferCount = state->frame_count;
  swap_chain_desc.Width = window_dim.width;
  swap_chain_desc.Height = window_dim.height;
  swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swap_chain_desc.SampleDesc.Count = 1;

  ComPtr<IDXGISwapChain1> swap_chain;
  if (FAILED(factory->CreateSwapChainForHwnd(state->command_queue.Get(), window_handle, &swap_chain_desc, nullptr, nullptr, &swap_chain))) {
    win32_print_str("dx_init(): CreateSwapChainForHwnd() failed\n");
  }
  if (FAILED(swap_chain.As(&state->swap_chain))) {
    win32_print_str("dx_init(): swap_chain.As() failed\n");
  }

  state->frame_index = state->swap_chain->GetCurrentBackBufferIndex();

  // Create Render Target View (RTV) descriptor heap
  D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
  rtv_heap_desc.NumDescriptors = state->frame_count;
  rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  if (FAILED(state->device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&state->rtv_heap)))) {
    win32_print_str("dx_init(): CreateDescriptorHeap()\n");
  }
  state->rtv_descriptor_size = state->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(state->rtv_heap->GetCPUDescriptorHandleForHeapStart());

  // Create a RTV for each frame.
  for (UINT n = 0; n < state->frame_count; n++) {
    if (FAILED(state->swap_chain->GetBuffer(n, IID_PPV_ARGS(&state->render_targets[n])))) {
      win32_print_str("dx_init(): GetBuffer() failed\n");
    }
    state->device->CreateRenderTargetView(state->render_targets[n].Get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, state->rtv_descriptor_size);

    if (FAILED(state->device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&state->command_allocators[n])))) {
      win32_print_str("dx_init(): CreateCommandAllocator() failed\n");
    }
  }    
  
  // Create synchronization objects and wait until assets have been uploaded to the GPU.  
  if (FAILED(state->device->CreateFence(state->fence_values[state->frame_index], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&state->fence)))) {
    win32_print_str("load_assets(): CreateFence() failed\n");
  }
  state->fence_values[state->frame_index]++;

  // Create an event handle to use for frame synchronization.
  state->fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  if (state->fence_event == nullptr) {
      HRESULT result = (HRESULT_FROM_WIN32(GetLastError()));
      if (FAILED(result)) win32_print_str("load_assets(): GetLastError() failed\n");
  }

  // Create Command List
  if (FAILED(state->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, state->command_allocators[state->frame_index].Get(), nullptr, IID_PPV_ARGS(&state->command_list)))) {
    win32_print_str("(win32) CreateCommandList() failed\n");
  }
  if (FAILED(state->command_list->Close())) {
    win32_print_str("(win32) Close() failed\n");
  }
}

internal bool8
dx_compile_shader(DX_Shader *shader) {
#ifdef DEBUG
  // Enable better shader debugging with the graphics debugging tools.
  UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
  UINT compileFlags = 0;
#endif
  //const char *file = (const char *)read_file_terminated("../shaders.hlsl").memory;
  LPCWSTR file = L"../shaders.hlsl";

  if (FAILED(D3DCompileFromFile(file, nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &shader->vertex, nullptr))) {
    win32_print_str("load_assets(): D3DCompileFromFile() failed\n");
    return 1;
  }
  if (FAILED(D3DCompileFromFile(file, nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &shader->pixel, nullptr))) {
    win32_print_str("load_assets(): D3DCompileFromFile() failed\n");
    return 1;
  }

  return 0;
}

internal void 
dx_create_pipeline(DX_State *state, DX_Pipeline *pipeline) {
  // Create an empty root signature
  CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc;
  root_signature_desc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  ComPtr<ID3DBlob> signature;
  ComPtr<ID3DBlob> error;
  if (FAILED(D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error))) {
    win32_print_str("dx_load_pipeline(): D3D12SerializeRootSignature() failed\n");
  }
  if (FAILED(state->device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&state->root_signature)))) {
    win32_print_str("load_assets(): CreateRootSignature() failed\n");
  }

  // Create pipeline state
  
  // Define the vertex input layout.
  D3D12_INPUT_ELEMENT_DESC input_element_descs[] =
  {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
  };

  // Describe and create the graphics pipeline state object (PSO).
  D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
  pso_desc.InputLayout = { input_element_descs, _countof(input_element_descs) };
  pso_desc.pRootSignature = state->root_signature.Get();
  pso_desc.VS = { reinterpret_cast<UINT8*>(pipeline->shader->vertex->GetBufferPointer()), pipeline->shader->vertex->GetBufferSize() };
  pso_desc.PS = { reinterpret_cast<UINT8*>(pipeline->shader->pixel->GetBufferPointer()), pipeline->shader->pixel->GetBufferSize() };
  pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
  pso_desc.DepthStencilState.DepthEnable = FALSE;
  pso_desc.DepthStencilState.StencilEnable = FALSE;
  pso_desc.SampleMask = UINT_MAX;
  pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  pso_desc.NumRenderTargets = 1;
  pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
  pso_desc.SampleDesc.Count = 1;
  if (FAILED(state->device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pipeline->state)))) {
    win32_print_str("dx_load_pipeline(): CreateGraphicsPipelineState() failed\n");
  }
}

// Wait for pending GPU work to complete.
internal void 
dx_wait_for_gpu(DX_State *state) {
    // Schedule a Signal command in the queue.
    if (FAILED(state->command_queue->Signal(state->fence.Get(), state->fence_values[state->frame_index]))) {
      win32_print_str("dx12_wait_for_gpu(): Signal() failed\n");
    }

    // Wait until the fence has been processed.
    if (FAILED(state->fence->SetEventOnCompletion(state->fence_values[state->frame_index], state->fence_event))) {
      win32_print_str("dx12_wait_for_gpu(): SetEventOnCompletion() failed\n");
    }
    WaitForSingleObjectEx(state->fence_event, INFINITE, FALSE);

    // Increment the fence value for the current frame.
    state->fence_values[state->frame_index]++;
}

internal void
dx_init_mesh(DX_State *state, Mesh *mesh) {
  u32 vertex_size = vertex_info_get_size(&mesh->vertex_info);
  const UINT vertex_buffer_size = vertex_size * mesh->vertices_count;

  // Note: using upload heaps to transfer static data like vert buffers is not 
  // recommended. Every time the GPU needs it, the upload heap will be marshalled 
  // over. Please read up on Default Heap usage. An upload heap is used here for 
  // code simplicity and because there are very few verts to actually transfer.
  HRESULT result = state->device->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
      D3D12_HEAP_FLAG_NONE,
      &CD3DX12_RESOURCE_DESC::Buffer(vertex_buffer_size),
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&mesh->gpu.vertex_buffer));
  if (FAILED(result)) {
    win32_print_str("load_assets(): CreateCommittedResource() failed");
  }

  // Copy the triangle data to the vertex buffer.
  u8* p_vertex_data_begin;
  CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
  result = mesh->gpu.vertex_buffer->Map(0, &readRange, reinterpret_cast<void**>(&p_vertex_data_begin));
  if (FAILED(result)) {
    win32_print_str("load_assets(): Map() failed");
  }
  memcpy(p_vertex_data_begin, mesh->vertices, vertex_buffer_size);
  mesh->gpu.vertex_buffer->Unmap(0, nullptr);

  // Initialize the vertex buffer view.
  mesh->gpu.vertex_buffer_view.BufferLocation = mesh->gpu.vertex_buffer->GetGPUVirtualAddress();
  mesh->gpu.vertex_buffer_view.StrideInBytes = vertex_size;
  mesh->gpu.vertex_buffer_view.SizeInBytes = vertex_buffer_size;

  // Wait for the command list to execute; we are reusing the same command 
  // list in our main loop but for now, we just want to wait for setup to 
  // complete before continuing.
  dx_wait_for_gpu(state);
}

internal void
dx12_draw_mesh(Mesh *mesh) {
  gfx.dx12.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  gfx.dx12.command_list->IASetVertexBuffers(0, 1, &mesh->gpu.vertex_buffer_view);
  gfx.dx12.command_list->DrawInstanced(3, 1, 0, 0);
}

internal void
dx12_set_viewport(u32 window_width, u32 window_height) {
  CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(window_width), static_cast<float>(window_height));
  gfx.dx12.command_list->RSSetViewports(1, &viewport);
}

internal void
dx12_set_scissor(s32 x, s32 y, u32 width, u32 height) {
  CD3DX12_RECT scissor = CD3DX12_RECT(0, 0, static_cast<LONG>(gfx.window_dim.width), static_cast<LONG>(gfx.window_dim.height));
  gfx.dx12.command_list->RSSetScissorRects(1, &scissor);
}

internal void
dx_start_frame(DX_State *state, DX_Pipeline *pipeline) {
  // Command list allocators can only be reset when the associated 
  // command lists have finished execution on the GPU; apps should use 
  // fences to determine GPU execution progress.
  if (FAILED(state->command_allocators[state->frame_index]->Reset())) {
    win32_print_str("dx_populate_command_list(): command allocator Reset() failed");
  }
  
  // However, when ExecuteCommandList() is called on a particular command 
  // list, that command list can then be reset at any time and must be before 
  // re-recording.
  if (FAILED(state->command_list->Reset(state->command_allocators[state->frame_index].Get(), pipeline->state.Get()))) {
    win32_print_str("dx_populate_command_list(): command list Reset() failed");
  }

  // Set necessary state.
  state->command_list->SetGraphicsRootSignature(state->root_signature.Get());

  // Indicate that the back buffer will be used as a render target.
  auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(state->render_targets[state->frame_index].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
  state->command_list->ResourceBarrier(1, &barrier);

  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(state->rtv_heap->GetCPUDescriptorHandleForHeapStart(), state->frame_index, state->rtv_descriptor_size);
  state->command_list->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
  
  const float clear_color[] = { 0.0f, 0.2f, 0.4f, 1.0f };
  state->command_list->ClearRenderTargetView(rtvHandle, clear_color, 0, nullptr);
}

internal void
dx_end_frame(DX_State *state) {
  // Indicate that the back buffer will now be used to present.
  auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(state->render_targets[state->frame_index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
  state->command_list->ResourceBarrier(1, &barrier);

  if (FAILED(state->command_list->Close())) {
    win32_print_str("dx_populate_command_list(): Close() failed");
  }
  
  // Execute the command list.
  ID3D12CommandList* pp_command_lists[] = { state->command_list.Get() };
  state->command_queue->ExecuteCommandLists(_countof(pp_command_lists), pp_command_lists);

  // Present the frame.
  if (FAILED(state->swap_chain->Present(1, 0))) {
    win32_print_str("dx_on_render(): Present() failed");
  }

  // Move to next frame
  // Schedule a Signal command in the queue.
  const u64 current_fence_value = state->fence_values[state->frame_index];
  if (FAILED(state->command_queue->Signal(state->fence.Get(), current_fence_value))) {
    win32_print_str("dx12_move_to_next_frame(): Signal() failed");
  }

  // Update the frame index.
  state->frame_index = state->swap_chain->GetCurrentBackBufferIndex();

  // If the next frame is not ready to be rendered yet, wait until it is ready.
  if (state->fence->GetCompletedValue() < state->fence_values[state->frame_index]) {
      if (FAILED(state->fence->SetEventOnCompletion(state->fence_values[state->frame_index], state->fence_event))) {
        win32_print_str("dx_move_to_next_frame(): SetEventOnCompletion() failed");
      }
      WaitForSingleObjectEx(state->fence_event, INFINITE, FALSE);
  }

  // Set the fence value for the next frame.
  state->fence_values[state->frame_index] = current_fence_value + 1;
}

internal void
dx_destroy(DX_State *state) {
  dx_wait_for_gpu(state);
  CloseHandle(state->fence_event);
}
