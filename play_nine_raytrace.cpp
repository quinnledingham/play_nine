// https://antongerdelan.net/opengl/raycasting.html
internal void
set_ray_coords(Ray *ray, Camera camera, Scene scene, Vector2_s32 mouse, Vector2_s32 screen_dim) {
    Vector3 ray_nds = {
        (2.0f * mouse.x) / screen_dim.width - 1.0f,
        (2.0f * mouse.y) / screen_dim.height - 1.0f,
        1.0f
    };

#ifdef VULKAN
    Vector4 ray_clip = {
        ray_nds.x,
        ray_nds.y,
        -1.0f,
        1.0f,
    };
#elif OPENGL
    Vector4 ray_clip = {
        ray_nds.x,
        -ray_nds.y,
        -1.0f,
        1.0f,
    };
#endif

    Vector4 ray_eye = m4x4_mul_v4(inverse(scene.projection), ray_clip);    
    Vector4 ray_world_v4 = m4x4_mul_v4(inverse(scene.view), ray_eye);
    Vector3 ray_world = ray_world_v4.xyz;
    ray_world = ray_world / ray_world_v4.w;

    ray->origin = ray_world;
    ray->direction = normalized(ray->origin - camera.position);

    //print("%f %f %f\n", ray->origin.x, ray->origin.y, ray->origin.z);
}

// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
internal Ray_Intersection
intersect_triangle(Ray ray, Triangle triangle) {    
    Vector3 edge1 = triangle.b - triangle.a;
    Vector3 edge2 = triangle.c - triangle.a;
    Vector3 ray_cross_e2 = cross_product(ray.direction, edge2);
    float32 det = dot_product(edge1, ray_cross_e2);

    if (det > -EPSILON && det < EPSILON) {
        return Ray_Intersection{}; // no intersection. this ray is parallel to this triangle
    }

    float32 inv_det = 1.0f / det;
    Vector3 s = ray.origin - triangle.a;
    float32 u = inv_det * dot_product(s, ray_cross_e2);
    if (u < 0 || u > 1) {
        return Ray_Intersection{}; // no intersection
    }

    Vector3 s_cross_e1 = cross_product(s, edge1);
    float32 v = inv_det * dot_product(ray.direction, s_cross_e1);

    if (v < 0 || u + v > 1) {
        return Ray_Intersection{}; // no intersection
    }

    // At this stage we can compute t to find out where the intersection point is on the line.
    float32 t = inv_det * dot_product(edge2, s_cross_e1);

    // ray intersection
    if (t > EPSILON) {
        Ray_Intersection p;
        Vector3 point = ray.origin + ray.direction * t;
        p.point.xyz = point;
        //p.normal = normalized(cross_product(edge1, edge2));
        //p.material = material;
        p.number_of_intersections = 1;
        return p;
    } else {
        // This means that there is a line intersection but not a ray intersection.
        return Ray_Intersection{}; // no intersection
    }
}

internal Ray_Intersection
intersect_triangle_array(Ray ray, Triangle_v4 *triangles, Matrix_4x4 model) {
    Ray_Intersection result = {};
    float32 min = 9999.9f;
    for (u32 i = 0; i < 204; i++) {
        Triangle_v4 triangle = triangles[i];

        Vector4 a = m4x4_mul_v4(model, { triangle.a.x, triangle.a.y, triangle.a.z, 1.0f });
        Vector4 b = m4x4_mul_v4(model, { triangle.b.x, triangle.b.y, triangle.b.z, 1.0f });
        Vector4 c = m4x4_mul_v4(model, { triangle.c.x, triangle.c.y, triangle.c.z, 1.0f });
    
        Triangle tri = { 
            a.xyz,
            b.xyz,
            c.xyz
        };

        Ray_Intersection intersection = intersect_triangle(ray, tri);
        float32 dist_point_to_origin = distance(intersection.point.xyz, ray.origin);
        if (intersection.number_of_intersections != 0 && dist_point_to_origin < min) {
            min = dist_point_to_origin;
            result = intersection;
        }
    }
    return result;
}

internal Ray_Intersection
intersect_triangle_mesh(Ray ray, Mesh *mesh, Matrix_4x4 model) {
    Ray_Intersection result = {};
    float32 min = 9999.9f;
    Vector3 test = {};
    for (u32 i = 0; i < mesh->indices_count; i += 3) {
        u32 i1 = mesh->indices[i + 0];
        u32 i2 = mesh->indices[i + 1];
        u32 i3 = mesh->indices[i + 2];
    
        Vertex_XNU v1 = ((Vertex_XNU *)mesh->vertices)[i1];
        Vertex_XNU v2 = ((Vertex_XNU *)mesh->vertices)[i2];
        Vertex_XNU v3 = ((Vertex_XNU *)mesh->vertices)[i3];

        Vector4 a = m4x4_mul_v4(model, { v1.position.x, v1.position.y, v1.position.z, 1.0f });
        Vector4 b = m4x4_mul_v4(model, { v2.position.x, v2.position.y, v2.position.z, 1.0f });
        Vector4 c = m4x4_mul_v4(model, { v3.position.x, v3.position.y, v3.position.z, 1.0f });
    
        Triangle triangle = { 
            a.xyz,
            b.xyz,
            c.xyz
        };

        Ray_Intersection intersection = intersect_triangle(ray, triangle);
        if (intersection.number_of_intersections != 0 && distance(intersection.point.xyz, ray.origin) < min) {
            min = distance(intersection.point.xyz, ray.origin);
            result = intersection;
        }

        test = triangle.c;
    }

    return result;
}

internal void
init_triangles(Model *model, Descriptor *desc) {
    u32 triangle_count = 0;
    for (u32 i = 0; i < model->meshes_count; i++) {
        Mesh *mesh = &model->meshes[i];
        for (u32 j = 0; j < mesh->indices_count; j += 3) {
            triangle_count++;
        }
    }

    Triangle_v4 *triangles = ARRAY_MALLOC(Triangle_v4, triangle_count);
    u32 triangle_index = 0;

    for (u32 i = 0; i < model->meshes_count; i++) {
        Mesh *mesh = &model->meshes[i];
        for (u32 j = 0; j < mesh->indices_count; j += 3) {
            u32 i1 = mesh->indices[j + 0];
            u32 i2 = mesh->indices[j + 1];
            u32 i3 = mesh->indices[j + 2];
        
            Vertex_XNU v1 = ((Vertex_XNU *)mesh->vertices)[i1];
            Vertex_XNU v2 = ((Vertex_XNU *)mesh->vertices)[i2];
            Vertex_XNU v3 = ((Vertex_XNU *)mesh->vertices)[i3];

            Triangle_v4 triangle = {};
            triangle.a.xyz = v1.position;
            triangle.b.xyz = v2.position;
            triangle.c.xyz = v3.position;

            triangles[triangle_index++] = triangle;
        }
    }

    
    GFX_Buffer buffer;
    buffer.size = triangle_count * sizeof(Triangle_v4);
    gfx.create_buffer(&buffer);
    memcpy((char*)buffer.data, (void *)triangles, buffer.size);

    *desc = gfx.descriptor_set(GFX_ID_RAY_TRIANGLE);
    gfx.set_storage_buffer(&buffer, *desc);
}

internal void
draw_ray(Ray *ray) {
    //draw_sphere(game->mouse_ray.origin, 0.0f, { 1, 1, 1 }, { 255, 0, 255, 1 });
    for (float32 i = 0; i < 30; i += 2.0f)
        gfx.draw_sphere(ray->origin + (ray->direction * i), 0.0f, { 1, 1, 1 }, { 0, 255, 0, 1 });
}

internal bool8
ray_model_intersection_cpu(Ray ray, Model *model, Matrix_4x4 card) {

    for (u32 i = 0; i < model->meshes_count; i++) {
        Ray_Intersection p = intersect_triangle_mesh(ray, &model->meshes[i], card);
        if (p.number_of_intersections != 0) {
            //print("card: %f %f %f\n", p.point.x, p.point.y, p.point.z);
            return true;
        }
    }

/*
    Ray_Intersection p = intersect_triangle_array(ray, NULL, card);
    if (p.number_of_intersections != 0) {
        print("card: %f %f %f\n", p.point.x, p.point.y, p.point.z);
        return true;
    }
*/
    return false;
}

internal void
mouse_ray_model_intersections_cpu(bool8 *selected, Ray mouse_ray, Game_Draw *draw, Model *card_model, u32 active_player) {
    for (u32 card_index = 0; card_index < 8; card_index++) {
        selected[card_index] = ray_model_intersection_cpu(mouse_ray, card_model, draw->hand_models[active_player][card_index]);
        if (selected[card_index]) return;
    }

    selected[GI_PICKUP_PILE] = ray_model_intersection_cpu(mouse_ray, card_model, draw->top_of_pile_model);
    if (draw->top_of_discard_pile_model.E[0] != 0)
        selected[GI_DISCARD_PILE] = ray_model_intersection_cpu(mouse_ray, card_model, draw->top_of_discard_pile_model);
}

#if VULKAN

GFX_Buffer output_buffer;

internal void
mouse_ray_model_intersections(bool8 selected[GI_SIZE], Ray mouse_ray, Descriptor *triangle_desc, Game_Draw *draw, Model *card_model, u32 active_player) {
    if (output_buffer.size == 0) {
        output_buffer.size = 500;
        gfx.create_buffer(&output_buffer);
    }
    
    gfx.start_compute();
    
    gfx.bind_shader(SHADER_RAY);
    
    //Descriptor tri_desc = render_get_descriptor_set(GFX_ID_RAY_TRIANGLE);
    
    Descriptor ray_desc = gfx.descriptor_set(GFX_ID_RAY_VERTEX);
    Descriptor out_desc = gfx.descriptor_set(GFX_ID_RAY_INTERSECTION);
    Descriptor object_desc = gfx.descriptor_set(GFX_ID_RAY_MODELS);
    
    Ray_v4 ray_v4 = {
        { mouse_ray.origin.x, mouse_ray.origin.y, mouse_ray.origin.z, 0.0f },
        { mouse_ray.direction.x, mouse_ray.direction.y, mouse_ray.direction.z, 0.0f },
    };

    gfx.update_ubo(ray_desc, &ray_v4);

    output_buffer.size = 10 * 48;
    gfx.set_storage_buffer(&output_buffer, out_desc);
    //vulkan_set_storage_buffer1(out_desc, 10 * 48);

    // load ubo buffer
    Matrix_4x4 object[10];
    for (u32 card_index = 0; card_index < 8; card_index++) {
        object[card_index] = draw->hand_models[active_player][card_index];
    }
    object[GI_PICKUP_PILE] = draw->top_of_pile_model;
    object[GI_DISCARD_PILE] = draw->top_of_discard_pile_model;

    char *test = (char*)vulkan_info.static_uniform_buffer.data + object_desc.offset;
    memcpy(test, object, sizeof(Matrix_4x4) * 10);

    // compute intersections
    
    // 48 is the size of Ray_Intersection in glsl
    for (u32 i = 0; i < 10; i++) {
        //Ray_Intersection *p = ((Ray_Intersection*)((u8*)vulkan_info.storage_buffer.data + out_desc.offset + (sizeof(Ray_Intersection) * i)));
        Ray_Intersection *p = ((Ray_Intersection*)((u8*)output_buffer.data + out_desc.offset + (sizeof(Ray_Intersection) * i)));
        if (p->number_of_intersections != 0) {
            //print("card: %f %f %f\n", p.point.x, p.point.y, p.point.z);
            selected[i] = true;
        } else {
            selected[i] = false;
        }
    }

    //memset((char*)vulkan_info.storage_buffer.data, 0, 10 * 48);
    memset((char *)output_buffer.data, 0, 10 * 48);

    gfx.bind_descriptor_set(*triangle_desc);
    
    gfx.bind_descriptor_set(ray_desc);
    gfx.bind_descriptor_set(out_desc);
    gfx.bind_descriptor_set(object_desc);

    gfx.dispatch(16, 1, 1);
    gfx.end_compute();

}

#endif // VULKAN
