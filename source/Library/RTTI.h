#pragma once
#include <string>
#include <cstdint>

namespace Library
{
	class RTTI
	{
	public:
		virtual const uintptr_t& TypeIdInstance() const = 0;

		virtual RTTI* QueryInterface(const uintptr_t id) const
		{
			return nullptr;
		}

		virtual bool Is(const uintptr_t id) const
		{
			return false;
		}

		virtual bool Is(const std::string& name) const
		{
			return false;
		}

		template <typename T>
		T* As() const
		{
			if (Is(T::TypeIdClass()))
			{
				return (T*)this;
			}

			return nullptr;
		}
	};

#define RTTI_DECLARATIONS(Type, ParentType)                                                              \
		public:                                                                                              \
			typedef ParentType Parent;                                                                       \
			static std::string TypeName() { return std::string(#Type); }                                     \
			virtual const uintptr_t& TypeIdInstance() const { return Type::TypeIdClass(); }               \
			static  const uintptr_t& TypeIdClass() { return sRunTimeTypeId; }                             \
			virtual Library::RTTI* QueryInterface( const uintptr_t id ) const                             \
			{                                                                                                \
				if (id == sRunTimeTypeId)                                                                    \
					{ return (RTTI*)this; }                                                                  \
				else                                                                                         \
					{ return Parent::QueryInterface(id); }                                                   \
			}                                                                                                \
			virtual bool Is(const uintptr_t id) const                                                     \
			{                                                                                                \
				if (id == sRunTimeTypeId)                                                                    \
					{ return true; }                                                                         \
				else                                                                                         \
					{ return Parent::Is(id); }                                                               \
			}                                                                                                \
			virtual bool Is(const std::string& name) const                                                   \
			{                                                                                                \
				if (name == TypeName())                                                                      \
					{ return true; }                                                                         \
				else                                                                                         \
					{ return Parent::Is(name); }                                                             \
			}                                                                                                \
	   private:                                                                                              \
			static uintptr_t sRunTimeTypeId;

#define RTTI_DEFINITIONS(Type) uintptr_t Type::sRunTimeTypeId = (uintptr_t)& Type::sRunTimeTypeId;
}