#include "translate.h"

Language g_languageId;

void setCurrentLanguage(const Language& languageId)
{
    g_languageId = languageId;
}


#include "translate_data.h"


const char* translate(const Translate& key)
{
    const char* t = g_translations[(u32)g_languageId][(u32)key];

    if (!t) {
        return "";
    }

    return t;
}