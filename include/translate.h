#pragma once

#include <switch.h>
#include <string>
#include <vector>

class Language
{
public:
    enum class Type : u8
    {
        None,
        en,
        cs,
        da,
        de,
        el,
        es,
        es_CO,
        es_AR,
        es_CL,
        es_PE,
        es_MX,
        es_US,
        fi,
        fr,
        fr_CA,
        fr_BE,
        hu,
        it,
        ja,
        ko,
        nl,
        no,
        pl,
        pt,
        pt_BR,
        sv,
        ru,
        zh,
        vi,
        tr,
        hi,
        he,
        id,
        tl,
        ar,
        fa,
        uk,
        hr
    };

    Language() : m_type(Type::en)
    {
    }

    Language(const Language& c)
    {
        m_type = c.m_type;
    }

    Language(const Type type)
    {
        m_type = type;
    }

    static std::vector<std::string>& list() {
        static std::vector<std::string> r;
        r.resize(0);

        for (int i = (int)Type::en; i <= (int)Type::zh; i++) {
            r.push_back(lookup(i));
        }
        return r;
    }

    Language(const std::string& s)
    {
        if (s == "en") { m_type = Type::en; }
        else if (s == "de") { m_type = Type::de; }
        else if (s == "es") { m_type = Type::es; }
        else if (s == "fr") { m_type = Type::fr; }
        else if (s == "it") { m_type = Type::it; }
        else if (s == "ja") { m_type = Type::ja; }
        else if (s == "ko") { m_type = Type::ko; }
        else if (s == "nl") { m_type = Type::nl; }
        else if (s == "pt") { m_type = Type::pt; }
        else if (s == "ru") { m_type = Type::ru; }
        else if (s == "zh") { m_type = Type::zh; }
        else if (s == "sv") { m_type = Type::sv; }
        else if (s == "pl") { m_type = Type::pl; }
        else if (s == "no") { m_type = Type::no; }
        else if (s == "hu") { m_type = Type::hu; }
        else if (s == "fi") { m_type = Type::fi; }
        else if (s == "el") { m_type = Type::el; }
        else if (s == "da") { m_type = Type::da; }
        else if (s == "cs") { m_type = Type::cs; }
        else if (s == "es-CO") { m_type = Type::es_CO; }
        else if (s == "es-AR") { m_type = Type::es_AR; }
        else if (s == "es-CL") { m_type = Type::es_CL; }
        else if (s == "es-PE") { m_type = Type::es_PE; }
        else if (s == "es-MX") { m_type = Type::es_MX; }
        else if (s == "es-US") { m_type = Type::es_US; }
        else if (s == "fr-CA") { m_type = Type::fr_CA; }
        else if (s == "fr-BE") { m_type = Type::fr_BE; }
        else if (s == "pt-BR") { m_type = Type::pt_BR; }
        else if (s == "vi") { m_type = Type::vi; }
        else if (s == "tr") { m_type = Type::tr; }
        else if (s == "hi") { m_type = Type::hi; }
        else if (s == "he") { m_type = Type::he; }
        else if (s == "id") { m_type = Type::id; }
        else if (s == "tl") { m_type = Type::tl; }
        else if (s == "ar") { m_type = Type::ar; }
        else if (s == "fa") { m_type = Type::fa; }
        else if (s == "uk") { m_type = Type::uk; }
        else if (s == "hr") { m_type = Type::hr; }
        else m_type = Type::None;
    }

    operator u8() const { return (u8)m_type; }

    const char* c_str() const {
        return lookup((int)m_type);
    }

    static const char* lookup(int i) {
        switch (i)
        {
        case (int)Type::en: return  "en";
        case (int)Type::de: return  "de";
        case (int)Type::es: return  "es";
        case (int)Type::fr: return  "fr";
        case (int)Type::it: return  "it";
        case (int)Type::ja: return  "ja";
        case (int)Type::ko: return  "ko";
        case (int)Type::nl: return  "nl";
        case (int)Type::pt: return  "pt";
        case (int)Type::ru: return  "ru";
        case (int)Type::zh: return  "zh";
        case (int)Type::sv: return "sv";
        case (int)Type::pl: return "pl";
        case (int)Type::no: return "no";
        case (int)Type::hu: return "hu";
        case (int)Type::fi: return "fi";
        case (int)Type::el: return "el";
        case (int)Type::da: return "da";
        case (int)Type::cs: return "cs";
        case (int)Type::es_CO: return "es-CO";
        case (int)Type::es_AR: return "es-AR";
        case (int)Type::es_CL: return "es-CL";
        case (int)Type::es_PE: return "es-PE";
        case (int)Type::es_MX: return "es-MX";
        case (int)Type::es_US: return "es-US";
        case (int)Type::fr_CA: return "fr-CA";
        case (int)Type::fr_BE: return "fr-BE";
        case (int)Type::pt_BR: return "pt-BR";
        case (int)Type::vi: return "vi";
        case (int)Type::tr: return "tr";
        case (int)Type::hi: return "hi";
        case (int)Type::he: return "he";
        case (int)Type::id: return "id";
        case (int)Type::tl: return "tl";
        case (int)Type::ar: return "ar";
        case (int)Type::fa: return "fa";
        case (int)Type::uk: return "uk";
        case (int)Type::hr: return "hr";
        }
        return "";
    }

    std::string str() {
        return std::string(c_str());
    }

protected:
    Type m_type;
};

#include "translate_defs.h"

const char* translate(const Translate& key);
void setCurrentLanguage(const Language& languageId);