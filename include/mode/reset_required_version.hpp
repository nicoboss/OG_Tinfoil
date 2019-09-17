#pragma once

#include <switch.h>
#include "mode/mode.hpp"

namespace tin::ui
{
    class ResetRequiredVersionMode : public IMode
    {
        public:
            ResetRequiredVersionMode();

            void OnSelected() override;
            void OnTitleIdSelected();
    };
}