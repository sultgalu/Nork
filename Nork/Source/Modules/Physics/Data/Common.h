#pragma once

namespace Nork::Physics
{
	struct Face
	{
		glm::vec3 norm;
		uint32_t vertIdx;
	};

	using index_t = uint32_t;
	struct Edge
	{
		union
		{
			index_t first;
			index_t x;
		};
		union
		{
			index_t second;
			index_t y;
		};

		index_t& operator[](uint32_t idx) const 
		{
			return ((index_t*)this)[idx];
		}
	};

	struct Collider
	{
		std::vector<glm::vec3> verts = {};
		std::vector<Edge> edges = {};
		std::vector<Face> faces = {};
		std::vector<std::vector<index_t>> faceVerts = {};
		glm::vec3 center = glm::vec3(0);
		bool isActive = false;

		static Collider Cube(glm::vec3 scale = glm::vec3(0));

		std::vector<uint32_t> SideFacesOfVert(uint32_t vertIdx) const
		{
			std::vector<uint32_t> res;
			for (size_t i = 0; i < faceVerts.size(); i++)
			{
				for (size_t j = 0; j < faceVerts[i].size(); j++)
				{
					if (faceVerts[i][j] == vertIdx)
					{
						res.push_back(i);
						break;
					}
				}
			}
			return res;
		}
		std::vector<uint32_t> EdgesOnFace(uint32_t faceIdx) const
		{
			std::vector<uint32_t> res;
			for (size_t i = 0; i < edges.size(); i++)
			{
				int count = 0;
				for (size_t j = 0; j < faceVerts[faceIdx].size(); j++)
				{
					if (faceVerts[faceIdx][j] == edges[i].first ||
						faceVerts[faceIdx][j] == edges[i].second)
					{
						if (++count == 2)
						{
							res.push_back(i);
							break;
						}
					}
				}
			}
			return res;
		}
		std::vector<Edge> Edges(const glm::vec3& vert) const
		{
			std::vector<Edge> result;

			for (size_t i = 0; i < edges.size(); i++)
			{
				if (verts[edges[i].first] == vert || verts[edges[i].second] == vert)
				{
					result.push_back(std::ref(edges[i]));
				}
			}

			return result;
		}
		inline std::pair<const glm::vec3&, const glm::vec3&> Vertices(const Edge& edge) const
		{
			return std::pair<const glm::vec3&, const glm::vec3&> { verts[edge[0]], verts[edge[1]] };
		}
		inline const glm::vec3& VertFromFace(const Face& face) const
		{
			return verts[face.vertIdx];
		}
		inline const glm::vec3& VertFromFace(uint32_t idx) const
		{
			return verts[faces[idx].vertIdx];
		}
		inline const glm::vec3& FirstVertFromEdge(const Edge& edge) const
		{
			return verts[edge[0]];
		}
		inline const glm::vec3& SecondVertFromEdge(const Edge& edge) const
		{
			return verts[edge[1]];
		}
		inline glm::vec3 EdgeDirection(const Edge& edge) const
		{
			return verts[edge[0]] - verts[edge[1]];
		}
		inline glm::vec3 EdgeMiddle(const Edge& edge) const
		{
			return (verts[edge[0]] + verts[edge[1]]) / 2.0f;
		}
	};

	enum class CollisionType : uint32_t
	{
		FaceVert, VertFace, EdgeEdge
	};

	struct CollisionResult
	{
		glm::vec3 dir;
		float depth;
		CollisionType type;
		uint16_t featureIdx1;
		uint16_t featureIdx2;
	};

	struct KinematicData
	{
		glm::vec3 position = glm::zero<glm::vec3>();
		glm::vec3 velocity = glm::zero<glm::vec3>();
		glm::vec3 forces = glm::zero<glm::vec3>();
		float mass = 1;
		
		glm::quat quaternion = glm::quat(0, 0, 0, 1);
		glm::vec3 w = glm::zero<glm::vec3>();
		glm::vec3 torque = glm::zero<glm::vec3>();
		float I = 1;
		
		bool isStatic = false;
		bool applyGravity = true;
		float elasticity = 0.2f;
		float friction = 0.8f;
	};

	class Object
	{
	public:
		Object(const KinematicData& kinem = KinematicData())
			: kinem(kinem) {}

		Object(const Collider& collider, const KinematicData& kinem = KinematicData())
			: localColl(collider), collider(collider), kinem(kinem)
		{
			UpdateInertia();
			UpdateCollider();
		}
		void UpdateCollider();
		void UpdateInertia() { this->kinem.I = CalcInertia(); }
	private:
		float CalcInertia();
	public:
		Collider localColl = Collider(); // const
		Collider collider = Collider();
		KinematicData kinem;
	};

	struct ObjectHandle
	{
		Object& Get() const;
		bool operator==(const ObjectHandle& other) const 
		{
			return handle == other.handle;
		}	
	public:
		uint64_t handle;
	};
}