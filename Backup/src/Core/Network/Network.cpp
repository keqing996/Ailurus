#include "Ailurus/Network/Network.h"
#include "Platform/PlatformApi.h"

namespace Ailurus
{
    namespace Network
    {
        void Initialize()
        {
            Npi::GlobalInit();
        }

        void Shutdown()
        {
            Npi::GlobalShutdown();
        }
    }
}
