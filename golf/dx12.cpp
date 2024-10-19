internal void
d3d12_get_hardware_adapter(IDXGIFactory1 *p_factory, IDXGIAdapter1 **pp_adapter, bool request_high_performance_adapter) {
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
d3d12_init(D3D12_State *state, HWND window_handle, Vector2_s32 window_dim) {
  state->frame_index = 0;
  state->rtv_descriptor_size = 0;
  
  // Enable the D3D12 debug layer
  UINT dxgi_factory_flags = 0;
#ifdef GOLF_DEBUG

  ComPtr<ID3D12Debug> debug_controller;
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)))) {
    debug_controller->EnableDebugLayer();
  }

  dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;

#endif // GOLF_DEBUG

  // Create the factory
  ComPtr<IDXGIFactory4> factory;
  if (FAILED(CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&factory)))) {
      win32_print_str("d3d12_init(): CreateDXGIFactory2() failed\n");
  }

  if (state->use_warp_device) {
    ComPtr<IDXGIAdapter> warp_adapter;
    if (FAILED(factory->EnumWarpAdapter(IID_PPV_ARGS(&warp_adapter)))) {
      win32_print_str("d3d12_init(): EnumWarpAdapter() failed");
    }

    if (FAILED(D3D12CreateDevice(warp_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&state->device)))) {
      win32_print_str("d3d12_init(): D3D12CreateDevice() warp adapter failed");
    }
  } else {
    ComPtr<IDXGIAdapter1> hardware_adapter;
    d3d12_get_hardware_adapter(factory.Get(), &hardware_adapter, true);

    if (FAILED(D3D12CreateDevice(hardware_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&state->device)))) {
      win32_print_str("d3d12_init(): D3D12CreateDevice() hardware adapter failed");
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
  if (!gfx.vsync)
    swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

  ComPtr<IDXGISwapChain1> swap_chain;
  if (FAILED(factory->CreateSwapChainForHwnd(state->command_queue.Get(), window_handle, &swap_chain_desc, nullptr, nullptr, &swap_chain))) {
    win32_print_str("d3d12_init(): CreateSwapChainForHwnd() failed\n");
  }
  if (FAILED(swap_chain.As(&state->swap_chain))) {
    win32_print_str("d3d12_init(): swap_chain.As() failed\n");
  }

  state->frame_index = state->swap_chain->GetCurrentBackBufferIndex();

  // Create Render Target View (RTV) descriptor heap
  D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
  rtv_heap_desc.NumDescriptors = state->frame_count;
  rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  if (FAILED(state->device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&state->rtv_heap)))) {
    win32_print_str("d3d12_init(): CreateDescriptorHeap()\n");
  }
  state->rtv_descriptor_size = state->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  // Describe and create a shader resource view (SRV) heap for the texture
  D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc = {};
  srv_heap_desc.NumDescriptors = 1;
  srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  if (FAILED(state->device->CreateDescriptorHeap(&srv_heap_desc, IID_PPV_ARGS(&state->srv_heap)))) {
    win32_print_str("d3d12_init(): srv CreateDescriptorHeap()\n");
  }
  
  // Create a RTV for each frame.
  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(state->rtv_heap->GetCPUDescriptorHandleForHeapStart());
  
  for (UINT n = 0; n < state->frame_count; n++) {
    if (FAILED(state->swap_chain->GetBuffer(n, IID_PPV_ARGS(&state->render_targets[n])))) {
      win32_print_str("d3d12_init(): GetBuffer() failed\n");
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
d3d12_compile_shader(D3D12_Shader *shader) {
#ifdef DEBUG
  // Enable better shader debugging with the graphics debugging tools.
  UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
  UINT compileFlags = 0;
#endif
  //const char *file = (const char *)read_file_terminated("../shaders.hlsl").memory;
  LPCWSTR file = L"../tex.hlsl";

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
d3d12_create_pipeline(D3D12_State *state, D3D12_Pipeline *pipeline) {
  // Create an empty root signature
  /*
  CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc;
  root_signature_desc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  ComPtr<ID3DBlob> signature;
  ComPtr<ID3DBlob> error;
  if (FAILED(D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error))) {
    win32_print_str("d3d12_load_pipeline(): D3D12SerializeRootSignature() failed\n");
  }
  if (FAILED(state->device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&state->root_signature)))) {
    win32_print_str("load_assets(): CreateRootSignature() failed\n");
  }
  */
  D3D12_FEATURE_DATA_ROOT_SIGNATURE feature_data = {};

  // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
  feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

  if (FAILED(state->device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &feature_data, sizeof(feature_data)))) {
    feature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
  }

  CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
  ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

  CD3DX12_ROOT_PARAMETER1 root_parameters[1];
  root_parameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

  D3D12_STATIC_SAMPLER_DESC sampler = {};
  sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
  sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  sampler.MipLODBias = 0;
  sampler.MaxAnisotropy = 0;
  sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
  sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
  sampler.MinLOD = 0.0f;
  sampler.MaxLOD = D3D12_FLOAT32_MAX;
  sampler.ShaderRegister = 0;
  sampler.RegisterSpace = 0;
  sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

  CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC root_signature_desc;
  root_signature_desc.Init_1_1(_countof(root_parameters), root_parameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  ComPtr<ID3DBlob> signature;
  ComPtr<ID3DBlob> error;
  if (FAILED(D3DX12SerializeVersionedRootSignature(&root_signature_desc, feature_data.HighestVersion, &signature, &error))) {
    win32_print_str("d3d12_load_pipeline(): D3D12SerializeRootSignature() failed\n");
  }
  if (FAILED(state->device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&state->root_signature)))) {
    win32_print_str("load_assets(): CreateRootSignature() failed\n");
  }

  // Create pipeline state
  
  // Define the vertex input layout.
  //D3D12_INPUT_ELEMENT_DESC input_element_descs[] = {
  //    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
  //    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
  //};

  D3D12_INPUT_ELEMENT_DESC input_element_descs[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
    win32_print_str("d3d12_load_pipeline(): CreateGraphicsPipelineState() failed\n");
  }
}

// Wait for pending GPU work to complete.
internal void 
d3d12_wait_for_gpu(D3D12_State *state) {
  // Schedule a Signal command in the queue.
  if (FAILED(state->command_queue->Signal(state->fence.Get(), state->fence_values[state->frame_index]))) {
    win32_print_str("d3d12_wait_for_gpu(): Signal() failed\n");
  }

  // Wait until the fence has been processed.
  if (FAILED(state->fence->SetEventOnCompletion(state->fence_values[state->frame_index], state->fence_event))) {
    win32_print_str("d3d12_wait_for_gpu(): SetEventOnCompletion() failed\n");
  }
  WaitForSingleObjectEx(state->fence_event, INFINITE, FALSE);

  // Increment the fence value for the current frame.
  state->fence_values[state->frame_index]++;
}

internal void
d3d12_init_mesh(D3D12_State *state, Mesh *mesh) {
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
  d3d12_wait_for_gpu(state);
}

internal HRESULT
d3d12_init_buffer(D3D12_Buffer *buffer, u32 size) {
  HRESULT hr = gfx.d3d12.device->CreateCommittedResource(
                 &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 
                 D3D12_HEAP_FLAG_NONE,
                 &CD3DX12_RESOURCE_DESC::Buffer(size),
                 D3D12_RESOURCE_STATE_GENERIC_READ,
                 nullptr,
                 IID_PPV_ARGS(&buffer->resource));

  if (SUCCEEDED(hr)) {
    void *data;
    // No CPU reads will be done from the resource
    CD3DX12_RANGE readRange(0, 0);
    buffer->resource->Map(0, &readRange, &data);
    buffer->data_cursor = buffer->data_begin = reinterpret_cast<u8*>(data);
    buffer->data_end = buffer->data_begin + size;
  }

  return hr;
}

u64 d3d12_align(u64 location, u64 align) {
  if ((0 == align) || (align & (align - 1))) {
    logprint("d3d12_align()", "non-pow2 alignment\n");
  }

  return ((location + (align - 1)) & ~(align - 1));
}

internal HRESULT
d3d12_suballocate_from_buffer(D3D12_Buffer *buffer, u32 size, u64 align) {
  buffer->data_cursor = reinterpret_cast<u8*>(d3d12_align(reinterpret_cast<u64>(buffer->data_cursor), align));
  return (buffer->data_cursor + size > buffer->data_end) ? E_INVALIDARG : S_OK;
}


ComPtr<ID3D12Resource> texture_upload_heap;

internal void
d3d12_init_bitmap(D3D12_Texture *texture, Bitmap *bitmap) {
  D3D12_RESOURCE_DESC texture_desc = {};
  texture_desc.MipLevels = 1;
  texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  texture_desc.Width = bitmap->width;
  texture_desc.Height = bitmap->height;
  texture_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
  texture_desc.DepthOrArraySize = 1;
  texture_desc.SampleDesc.Count = 1;
  texture_desc.SampleDesc.Quality = 0;
  texture_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

  HRESULT hr = gfx.d3d12.device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
    D3D12_HEAP_FLAG_NONE,
    &texture_desc,
    D3D12_RESOURCE_STATE_COPY_DEST,
    nullptr,
    IID_PPV_ARGS(&texture->resource));
  if (FAILED(hr)) {
    logprint("d3d12_init_bitmap()", "failed to create committed resource\n");
  }

  const u64 upload_buffer_size = GetRequiredIntermediateSize(texture->resource.Get(), 0, 1);

  // create the CPU upload buffer
  hr = gfx.d3d12.device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(upload_buffer_size),
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&texture_upload_heap));
  if (FAILED(hr)) {
    logprint("d3d12_init_bitmap()", "failed to create upload buffer\n");
  }

  // copy data to the intermediate upload heap and then schedule a copy from the upload heap to the texture2D
  D3D12_SUBRESOURCE_DATA texture_data = {};
  texture_data.pData = bitmap->memory;
  texture_data.RowPitch = bitmap->pitch;
  texture_data.SlicePitch = texture_data.RowPitch * bitmap->height;

  UpdateSubresources(gfx.d3d12.command_list.Get(), texture->resource.Get(), texture_upload_heap.Get(), 0, 0, 1, &texture_data);
  gfx.d3d12.command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture->resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

  // Describe and create a srv for the texture
  D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
  srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srv_desc.Format = texture_desc.Format;
  srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  srv_desc.Texture2D.MipLevels = 1;
  gfx.d3d12.device->CreateShaderResourceView(texture->resource.Get(), &srv_desc, gfx.d3d12.srv_heap->GetCPUDescriptorHandleForHeapStart());
}

internal void
d3d12_draw_mesh(Mesh *mesh) {
  gfx.d3d12.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  gfx.d3d12.command_list->IASetVertexBuffers(0, 1, &mesh->gpu.vertex_buffer_view);
  gfx.d3d12.command_list->DrawInstanced(6, 1, 0, 0);
}

internal void
d3d12_set_viewport(u32 window_width, u32 window_height) {
  CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(window_width), static_cast<float>(window_height));
  gfx.d3d12.command_list->RSSetViewports(1, &viewport);
}

internal void
d3d12_set_scissor(s32 x, s32 y, u32 width, u32 height) {
  CD3DX12_RECT scissor = CD3DX12_RECT(0, 0, static_cast<LONG>(gfx.window_dim.width), static_cast<LONG>(gfx.window_dim.height));
  gfx.d3d12.command_list->RSSetScissorRects(1, &scissor);
}

internal void
d3d12_clear_color(Vector4 color) {
  
}

internal void
d3d12_bind_pipeline(D3D12_Pipeline *pipeline) {
  gfx.d3d12.command_list->SetPipelineState(pipeline->state.Get());
}

internal void
d3d12_start_commands() {
  D3D12_State *state = &gfx.d3d12;
  // Command list allocators can only be reset when the associated 
  // command lists have finished execution on the GPU; apps should use 
  // fences to determine GPU execution progress.
  if (FAILED(state->command_allocators[state->frame_index]->Reset())) {
    win32_print_str("d3d12_start_frame(): command allocator Reset() failed\n");
  }
  
  // However, when ExecuteCommandList() is called on a particular command 
  // list, that command list can then be reset at any time and must be before 
  // re-recording.
  if (FAILED(state->command_list->Reset(state->command_allocators[state->frame_index].Get(), nullptr))) {
    win32_print_str("d3d12_start_frame(): command list Reset() failed\n");
  }
  
  state->command_list->SetGraphicsRootSignature(state->root_signature.Get());
}

internal void
d3d12_end_commands() {
  D3D12_State *state = &gfx.d3d12;
  if (FAILED(state->command_list->Close())) {
    win32_print_str("d3d12_populate_command_list(): Close() failed");
  }
  
  // Execute the command list.
  ID3D12CommandList* pp_command_lists[] = { state->command_list.Get() };
  state->command_queue->ExecuteCommandLists(_countof(pp_command_lists), pp_command_lists);
}

internal void
d3d12_start_frame() {
  D3D12_State *state = &gfx.d3d12;
  
  // Command list allocators can only be reset when the associated 
  // command lists have finished execution on the GPU; apps should use 
  // fences to determine GPU execution progress.
  if (FAILED(state->command_allocators[state->frame_index]->Reset())) {
    win32_print_str("d3d12_start_frame(): command allocator Reset() failed\n");
  }
  
  // However, when ExecuteCommandList() is called on a particular command 
  // list, that command list can then be reset at any time and must be before 
  // re-recording.
  if (FAILED(state->command_list->Reset(state->command_allocators[state->frame_index].Get(), nullptr))) {
    win32_print_str("d3d12_start_frame(): command list Reset() failed\n");
  }

  // Set necessary state.
  state->command_list->SetGraphicsRootSignature(state->root_signature.Get());

  // Indicate that the back buffer will be used as a render target.
  auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(state->render_targets[state->frame_index].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
  state->command_list->ResourceBarrier(1, &barrier);

  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(state->rtv_heap->GetCPUDescriptorHandleForHeapStart(), state->frame_index, state->rtv_descriptor_size);
  state->command_list->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
  
  const float clear_color[] = { 0.0f, 0.2f, 0.4f, 1.0f };
  gfx.d3d12.command_list->ClearRenderTargetView(rtvHandle, clear_color, 0, nullptr);

  ID3D12DescriptorHeap* heaps[] = { gfx.d3d12.srv_heap.Get() };
  gfx.d3d12.command_list->SetDescriptorHeaps(_countof(heaps), heaps);
  gfx.d3d12.command_list->SetGraphicsRootDescriptorTable(0, gfx.d3d12.srv_heap->GetGPUDescriptorHandleForHeapStart());
}

internal void
d3d12_end_frame() {
  D3D12_State *state = &gfx.d3d12;
  
  // Indicate that the back buffer will now be used to present.
  auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(state->render_targets[state->frame_index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
  state->command_list->ResourceBarrier(1, &barrier);

  if (FAILED(state->command_list->Close())) {
    win32_print_str("d3d12_populate_command_list(): Close() failed");
    win32_print_last_error();
  }
  
  // Execute the command list.
  ID3D12CommandList* pp_command_lists[] = { state->command_list.Get() };
  state->command_queue->ExecuteCommandLists(_countof(pp_command_lists), pp_command_lists);

  // Present the frame.
  u32 present_flags = 0;
  if (!gfx.vsync)
    present_flags |= DXGI_PRESENT_ALLOW_TEARING;
  
  if (FAILED(state->swap_chain->Present(0, present_flags))) {
    win32_print_str("d3d12_on_render(): Present() failed\n");
    win32_print_last_error();
  }

  // Move to next frame
  // Schedule a Signal command in the queue.
  const u64 current_fence_value = state->fence_values[state->frame_index];
  if (FAILED(state->command_queue->Signal(state->fence.Get(), current_fence_value))) {
    win32_print_str("d3d12_move_to_next_frame(): Signal() failed");
  }

  // Update the frame index.
  state->frame_index = state->swap_chain->GetCurrentBackBufferIndex();

  // If the next frame is not ready to be rendered yet, wait until it is ready.
  if (state->fence->GetCompletedValue() < state->fence_values[state->frame_index]) {
      if (FAILED(state->fence->SetEventOnCompletion(state->fence_values[state->frame_index], state->fence_event))) {
        win32_print_str("d3d12_move_to_next_frame(): SetEventOnCompletion() failed");
      }
      WaitForSingleObjectEx(state->fence_event, INFINITE, FALSE);
  }

  // Set the fence value for the next frame.
  state->fence_values[state->frame_index] = current_fence_value + 1;
}

internal void
d3d12_destroy(D3D12_State *state) {
  d3d12_wait_for_gpu(state);
  CloseHandle(state->fence_event);
}
