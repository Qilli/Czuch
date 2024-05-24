#pragma once
#include "./Subsystems/BaseSubsystem.h"


namespace Czuch
{
    class CZUCH_API UIBaseManager : public BaseSubsystem<UIBaseManager>
    {
    public:
        virtual void Init() override {};
        virtual void Shutdown() override {};
        virtual void Update(TimeDelta timeDelta) override {};
        virtual void Draw(){};
    };
}


