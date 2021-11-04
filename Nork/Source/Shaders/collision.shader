#type compute

#version 330 core

// group.x -> face1 (x)
// group.y -> face2 (x)
// invoke.x -> edge on face2 (3)

layout(local_size_x = 3​) in;

struct Shape
{
	uint vertOffs, vertCount;
	uint faceOffs, faceCount;
};

layout(std430) buffer input
{
	uint vertSize;
	uint faceSize;

	Shape shapes[1000];
	vec3 verts[8 * 1000];
	uvec3 faces[12 * 1000];
	vec3 fNorms[12 * 1000];

	bool results[1000 * 1000];
};

void face()
{
	Shape shape1 = shapes[gl_WorkGroupID.x];
	Shape shape2 = shapes[gl_WorkGroupID.y];

	for (size_t i = 0; i < shape1.faces.size(); i++)
	{
		for (uint i = 0; i < length; i++)
		{

		}
		glm::vec3& farthestVertTowardsFace = Farthest(shape2.verts, -shape1.fNorm[i]);
		float pointDistanceFromFaceOutwards = SignedDistance(shape1.fNorm[i], shape1.SomePointOnFace(shape1.faces[i]), farthestVertTowardsFace);
		if (pointDistanceFromFaceOutwards > state.highest)
		{
			if (resType == 0)
			{
				state.faceAndVert = std::pair(&shape1.faces[i], &farthestVertTowardsFace);
			}
			else
			{
				state.vertAndFace = std::pair(&farthestVertTowardsFace, &shape1.faces[i]);
			}
			state.highest = pointDistanceFromFaceOutwards;
			state.resType = resType;

			if (pointDistanceFromFaceOutwards > 0)
				return false; // no collision, but set a separating face,
		}
	}
	return true;

}

void main()
{
	pos = worldPos;
	diffuse_spec = colliding > 0 ? vec3(1.0f, 0.0f, 0.0f) : texture(materialTex.diffuse, texCoord).rgb;
	specular = 1 - texture(materialTex.roughness, texCoord).r;
	//diffuse_spec = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	vec3 norm = texture(materialTex.normals, texCoord).rgb;
	norm = norm * 2.0f - 1.0f; // [0;1] -> [-1;1]
	normal = normalize(TBN * norm); // transforming from tangent-space -> world space

	// normal = texture(materialTex.normals, texCoord).rgb;
	// normal = normalize(vNorm);
}