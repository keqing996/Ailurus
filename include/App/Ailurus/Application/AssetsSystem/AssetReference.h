#pragma once

#include <concepts>
#include "Asset.h"

namespace Ailurus
{
	template <class T> requires std::derived_from<T, Asset>
	class AssetReference
	{
	public:
		explicit AssetReference(T* pAssets)
			: _pAsset(pAssets)
		{
			if (_pAsset != nullptr)
				_pAsset->AddRef();
		};

		AssetReference(const AssetReference& other)
			: _pAsset(other._pAsset)
		{
			if (_pAsset != nullptr)
				_pAsset->AddRef();
		}

		AssetReference(AssetReference&& other) noexcept
			: _pAsset(other._pAsset)
		{
			other._pAsset = nullptr;
		}

		AssetReference& operator=(const AssetReference& other)
		{
			if (this != &other)
			{
				_pAsset = other._pAsset;
				if (_pAsset != nullptr)
					_pAsset->AddRef();
			}
			return *this;
		}

		AssetReference& operator=(AssetReference&& other) noexcept
		{
			if (this != &other)
			{
				_pAsset = other._pAsset;
				other._pAsset = nullptr;
			}

			return *this;
		}

		~AssetReference()
		{
			if (_pAsset != nullptr)
				_pAsset->RemoveRef();
		}

		explicit operator bool() const noexcept
		{
			return _pAsset != nullptr;
		}

		bool operator==(std::nullptr_t) const noexcept
		{
			return _pAsset == nullptr;
		}

		bool operator!=(std::nullptr_t) const noexcept
		{
			return _pAsset != nullptr;
		}

	public:
		T* Get() const
		{
			return reinterpret_cast<T*>(_pAsset);
		}

	private:
		Asset* _pAsset = nullptr;
	};
} // namespace Ailurus