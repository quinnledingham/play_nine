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
        p.point = point;
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
    
        Triangle triangle = {};
        triangle.a = a.xyz;
        triangle.b = b.xyz;
        triangle.c = c.xyz;

        Ray_Intersection intersection = intersect_triangle(ray, triangle);
        if (intersection.number_of_intersections != 0 && distance(intersection.point, ray.origin) < min) {
            min = distance(intersection.point, ray.origin);
            result = intersection;
        }

        test = triangle.c;
    }

    return result;
}

internal bool8
ray_mesh_intersection_cpu(Ray ray, Mesh *hitbox, Transform t) {

    Matrix_4x4 t_matrix = m4x4(t);
    Ray_Intersection p = intersect_triangle_mesh(ray, hitbox, t_matrix);
    if (p.number_of_intersections != 0) {
        //print("card: %f %f %f\n", p.point.x, p.point.y, p.point.z);
        return true;
    }

    return false;

}

// https://antongerdelan.net/opengl/raycasting.html
internal void
set_ray_coords(Ray *ray, Camera camera, Scene scene, Vector2 mouse, Vector2_s32 screen_dim) {
    Vector3 ray_nds = {
        (2.0f * mouse.x) / screen_dim.width - 1.0f,
        (2.0f * mouse.y) / screen_dim.height - 1.0f,
        1.0f
    };

#ifdef GFX_VULKAN
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

internal void
draw_ray(Ray *ray) {
    Descriptor_Set local_desc_set = gfx_descriptor_set(GFXID_LOCAL);

    Descriptor color_desc = gfx_descriptor(&local_desc_set, 0);
    Local local = {};
    local.color = { 0, 255, 0, 1 };
    vulkan_update_ubo(color_desc, (void *)&local);

    vulkan_bind_descriptor_set(local_desc_set);

    vulkan_bind_mesh(&draw_ctx.sphere);

    for (float32 i = 0; i < 30; i += 2.0f) {
        Vector3 coords = ray->origin + (ray->direction * i);
        Object object = {};
        object.model = create_transform_m4x4(coords, get_rotation(0, {0, 1, 0}), {1, 1, 1});
        vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
        vkCmdDrawIndexed(VK_CMD, draw_ctx.sphere.indices_count, 1, 0, 0, 0);
    }
}