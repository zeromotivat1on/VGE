#include "VgeApplication.h"

std::unique_ptr<vge::VgeApplication> vge::CreateVgeApplication()
{
    return std::make_unique<VgeApplication>();
}
