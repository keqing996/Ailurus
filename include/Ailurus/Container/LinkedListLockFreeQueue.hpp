#pragma once
#include <atomic>
#include <memory>
#include <optional>
#include "../Utility/SpinPause.h"

namespace Ailurus
{
    template <typename T>
    class LinkedListLockFreeQueue
    {
        static constexpr uint32_t CACHE_LINE = 64;

    public:
        struct Node
        {
            T data;
            Node* pNext;

            explicit Node(T&& t): data(t), pNext(nullptr) { }
        };

        template<typename D = T>
        void Enqueue(D&& data)
        {
            Node pNewNode = new Node(std::forward<D>(data));

            Node* currentTail = _pTail.load();
            while (true)
            {
                if (_pTail.compare_exchange_weak(currentTail, pNewNode))
                    break;

                SpinPause();
            }

            if (currentTail != nullptr)
                currentTail->pNext = pNewNode;
        }


    private:
        alignas(CACHE_LINE) std::atomic<Node*> _pHead { nullptr };
        alignas(CACHE_LINE) std::atomic<Node*> _pTail { nullptr };
    };
}