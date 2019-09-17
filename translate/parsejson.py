import os
import os.path
import re
import json
from googletrans import Translator
from binascii import hexlify as hx, unhexlify as uhx

translator = Translator(service_urls=['translate.google.com'])

try:
	os.mkdir('cache')
except:
	pass

def doTranslate(lang, key, text):
	cacheFile = 'cache/%s.%s.txt' % (lang, key)
	
	if lang == 'zh':
		lang = 'zh-CN'
	
	if os.path.isfile(cacheFile):
		try:
			with open(cacheFile, 'r', encoding='utf-8') as f:
				return f.read() or None
		except:
			return None
			
	text = text.replace('\\n', '\n')
	translations = translator.translate([text], dest=lang)
	print('tranlating to ' + lang)
	try:
		for translation in translations:
			translatedText = translation.text.replace('\n', '\\n')
			
			with open(cacheFile, 'w', encoding='utf-8') as f:
				f.write(translatedText)
			return translatedText or None
	except BaseException as e:
		print(str(e))
		
	return None
	

with open('translations/translate.json', encoding="utf8") as f:
	t = json.loads(f.read())

inv = {}

langs = ['None', 'en', 'cs', 'da', 'de', 'el', 'es', 'es-CO', 'es-AR', 'es-CL', 'es-PE', 'es-MX', 'es-US', 'fi', 'fr', 'fr-CA', 'fr-BE', 'hu', 'it', 'ja', 'ko', 'nl', 'no', 'pl', 'pt', 'pt-BR', 'sv', 'ru', 'zh', 'vi', 'tr', 'hi', 'he', 'id', 'tl', 'ar', 'fa', 'uk', 'hr']

blacklist = ['NSP_INSTALL_EXTRACTED_FAIL', 'NSP_INSTALL_NETWORK_FROM']

for lang, keys in t.items():
	for key,text in keys.items():
		if key in blacklist:
			continue
		if not key in inv:
			inv[key] = {}
			
		inv[key][lang] = text
		
t = {}

for lang in langs:
	t[lang] = {}
	
for key, trans in inv.items():
	if key in blacklist:
		continue
	for lang in langs:
		if not lang in trans:
			t[lang][key] = None
		else:
			t[lang][key] = trans[lang]
	
	
with open('translations/translate.json', mode="w", encoding="utf-8") as f:
	f.write(json.dumps(t, sort_keys=True, indent=4, ensure_ascii=False))

def parseLang(lang):
	if '-' not in lang:
		return [lang, None]

	return lang.split('-')
	
for key, trans in inv.items():
	if key in blacklist:
		continue
	for lang in langs:
		langKey, regionKey = parseLang(lang)
		translatedText = None
		
		if not lang in trans or trans[lang] is None:
			if langKey in trans and trans[langKey] is not None:
				translatedText = trans[langKey]
				t[lang][key] = translatedText
				continue
		
		
		if not lang in trans or trans[lang] is None:
			if lang != 'None':
				try:
					translatedText = doTranslate(langKey, key, inv[key]['en'])
				except BaseException as e:
					print(str(e))
				print('missing %s - %s' % (lang, inv[key]['en']))
				
			if translatedText is None:
				t[lang][key] = inv[key]['en']
			else:
				t[lang][key] = translatedText
		else:
			t[lang][key] = trans[lang]


first = True
print('enum class Language\n{')
for lang, keys in t.items():
	if first:
		first = False
		print('    ' + lang.upper() + ' = 0,')
	else:
		print('    ' + lang.upper() + ',')
print('    LAST')
print('}\n')





with open('../include/translate_defs.h', mode="w", encoding="utf-8") as f:
	first = True
	f.write('enum class Translate\n{\n')
	for key, j in keys.items():
		if first:
			first = False
			f.write('    ' + key + ' = 0,\n')
		else:
			f.write('    ' + key + ',\n')
	f.write('    LAST\n')
	f.write('};\n')




def getBytes(x):
	s = bytes(x, 'utf-8')
	r = ''
	for c in s:
		r += '\\x' + '{:02x}'.format(c)
	return r


with open('../include/translate_data.h', mode="w", encoding="utf-8") as f:
	i = 0
	f.write('const char* g_translations[%d][%d] = {\n' % (len(t)+2, len(keys)))
	for lang,keys in t.items():
		if i > 0:
			f.write('    ,\n')
		f.write('    {\n')
		for key,text in keys.items():
			if text is not None:
				f.write('        "%s", // %s\n' % (getBytes(str(text.replace('"', '\\"'))), key))
		
		f.write('    }\n')
		i += 1
	f.write('};\n')
