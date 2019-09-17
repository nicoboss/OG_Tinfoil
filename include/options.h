#pragma once

#include <string>
#include "translate.h"

class OptionsContext
{
public:
    OptionsContext();
    virtual ~OptionsContext();

    bool Load();
    bool Save();

    const bool GetIgnoreFirmwareVersion() const { return m_ignoreFirmwareVersion; }
    const Language& GetLanguage() const { return m_language; }
protected:

    bool m_ignoreFirmwareVersion;
    Language m_language;
};

const OptionsContext& Options();