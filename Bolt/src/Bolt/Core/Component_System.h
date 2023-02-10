#pragma once
#include "Assert.h"

#include <array>
#include <string>
#include <vector>
#include <unordered_map>

namespace Bolt
{
	constexpr uint32_t CMPT_BUCKET_SIZE = 10;

	typedef uint32_t Entity_ID;
	
	template<typename T>
	struct CMPT_Bucket 
	{
		struct Internal_Data
		{
			char bits[sizeof(T)];
		};

		CMPT_Bucket() {}
		CMPT_Bucket(const CMPT_Bucket& other) = delete;
		CMPT_Bucket(CMPT_Bucket&& other) = delete;

		bool Full() { return m_size == CMPT_BUCKET_SIZE; }
		T* Emplace_Back() 
		{
			ASSERT(!Full(), "Bucket is full!");
			return ((T*)&m_components[m_size++]);
		}

	private:
		uint32_t m_size = 0;
		std::array<Internal_Data, CMPT_BUCKET_SIZE> m_components;

	};

	class CMPT_Pool_Base {};

	template<typename T>
	class CMPT_Pool : public CMPT_Pool_Base
	{
	private:
		struct Element
		{
			uint32_t index = 0;
			T* ptr = nullptr;
		};

	public:
		CMPT_Pool() { Create_New_Bucket(); }
		~CMPT_Pool()
		{
			for (CMPT_Bucket<T>*& bucket : m_buckets)
				delete bucket;
		}

		const std::vector<T*>& Components() { return m_living_components; }

		template<class... Args>
		T& Attach(Entity_ID id, Args... args)
		{
			ASSERT(id, "Invalid entity ID!");
			ASSERT(Find(id) == nullptr, std::string("Entity [") + std::to_string(id) + "] already has component of type: " + typeid(T).name());

			T* element;
			if(m_dead_components.empty())
			{
				if (Top_Bucket().Full())
					Create_New_Bucket();
				
				element = Top_Bucket().Emplace_Back();
			}
			else
			{
				element = m_dead_components.back();
				m_dead_components.pop_back();
			}
		
			new (element) T(args...);

			m_entity_map[id] = { (uint32_t)m_living_components.size(), element };
			m_id_map[element] = id;
			m_living_components.push_back(element);

			return *element;
		}

		void Detach(Entity_ID id)
		{
			Element element = Find_Element(id);
			if (!element.ptr)
			{
				WARN(std::string("Entity [") + std::to_string(id) + "] doens't have component of type: " + typeid(T).name());
				return;
			}

			element.ptr->~T();

			m_id_map.erase(element.ptr);
			m_entity_map.erase(id);

			T* top_living = m_living_components.back();
			Entity_ID top_id = m_id_map[top_living];
			Element top_element = m_entity_map[top_id];

			m_living_components[element.index] = top_living;
			m_entity_map[top_id] = { element.index, top_element.ptr };
			
			m_living_components.pop_back();
			m_dead_components.push_back(element.ptr);
		}

		T* Find(Entity_ID id)
		{
			ASSERT(id, "Invalid entity ID!");
			
			Element element = Find_Element(id);
			if (element.ptr) return element.ptr;
			return nullptr;
		}

		T& Get(Entity_ID id)
		{
			ASSERT(id, "Invalid entity ID!");
			T* element = Find(id);
			ASSERT(element, std::string("Entity [") + std::to_string(id) + "] doens't have component of type: " + typeid(T).name());
			return *element;
		}

		Entity_ID Get_Entity_ID(T* element)
		{
			ASSERT(element, "Entity pointer is a nullptr!");
			auto iterator = m_id_map.find(element);
			if (iterator != m_id_map.end())
				return iterator->second;

			ERROR("Element does not have an entity ID!");
		}

	private:
		CMPT_Bucket<T>& Top_Bucket() { return *m_buckets.back(); }
		void Create_New_Bucket() { m_buckets.push_back(new CMPT_Bucket<T>()); }

		Element Find_Element(Entity_ID id)
		{
			auto iterator = m_entity_map.find(id);
			if (iterator != m_entity_map.end())
				return iterator->second;

			return {};
		}

	private:
		std::vector<T*> m_dead_components;
		std::vector<T*> m_living_components;
		std::vector<CMPT_Bucket<T>*> m_buckets;
		std::unordered_map<Entity_ID, Element> m_entity_map;
		std::unordered_map<T*, Entity_ID> m_id_map;
	};

	struct Entity;

	class CMPT_Pools
	{
		friend Entity;
	
	public:
		template<typename T>
		const std::vector<T*>& Components() { return Pool<T>().Components(); }

		template<typename T>
		Entity_ID Get_Entity_ID(T* element) { return Pool<T>().Get_Entity_ID(element); }

	private:
		template<typename T>
		CMPT_Pool<T>& Pool()
		{
			CMPT_Pool_Base*& pool = m_pools[typeid(T).name()];
			if (!pool) pool = new CMPT_Pool<T>();
			return *((CMPT_Pool<T>*)pool);
		}

	private:
		std::unordered_map<const char*, CMPT_Pool_Base*> m_pools;
	};

	class Entity_Dispatcher;

	struct Entity
	{
		Entity() = default;

		friend Entity_Dispatcher;

		template<typename T, class... Args>
		T& Attach(Args... args) { return Pool<T>().Attach(m_id, args...); }

		template<typename T>
		void Detach() { Pool<T>().Detach(m_id); }

		template<typename T>
		T* Find() { return Pool<T>().Find(m_id); }

		template<typename T>
		T& Get() { return Pool<T>().Get(m_id); }

		Entity_ID ID() { return m_id; }

	private:
		template<typename T>
		CMPT_Pool<T>& Pool() 
		{ 
			ASSERT(m_pools, "Pools is a nullptr");
			return m_pools->Pool<T>(); 
		}

		Entity(Entity_ID id, CMPT_Pools* pools) : m_id(id), m_pools(pools) {}

	private:
		Entity_ID m_id = 0;
		CMPT_Pools* m_pools = nullptr;
	};

	class Entity_Dispatcher
	{
	public:
		Entity_Dispatcher(CMPT_Pools& pools) : m_pools(pools) {}

		Entity Create() { return Entity(m_next_id++, &m_pools); }
		Entity Create(Entity_ID id) { return Entity(id, &m_pools); };
		void Create(Entity& entity) { entity.m_id = m_next_id++; entity.m_pools = &m_pools; };

	private:
		CMPT_Pools& m_pools;
		Entity_ID m_next_id = 1;
	};
}