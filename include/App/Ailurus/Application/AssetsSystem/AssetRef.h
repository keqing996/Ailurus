#pragma once

#include <concepts>
#include "Asset.h"

namespace Ailurus
{
	template <class T> requires std::derived_from<T, Asset>
	class AssetRef
	{
	public:
		explicit AssetRef(T* pAssets)
			: _pAsset(pAssets)
		{
			if (_pAsset != nullptr)
				_pAsset->AddRef();
		};

		AssetRef(const AssetRef& other)
			: _pAsset(other._pAsset)
		{
			if (_pAsset != nullptr)
				_pAsset->AddRef();
		}

		AssetRef(AssetRef&& other) noexcept
			: _pAsset(other._pAsset)
		{
			other._pAsset = nullptr;
		}

		AssetRef& operator=(const AssetRef& other)
		{
			if (this != &other)
			{
				_pAsset = other._pAsset;
				if (_pAsset != nullptr)
					_pAsset->AddRef();
			}
			return *this;
		}

		AssetRef& operator=(AssetRef&& other) noexcept
		{
			if (this != &other)
			{
				_pAsset = other._pAsset;
				other._pAsset = nullptr;
			}

			return *this;
		}

		~AssetRef()
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