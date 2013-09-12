#include <stdlib.h>
#include <stdio.h>
#include "menu.h"
#include "mplayer_func.h"
#include "mplayer_common.h"
#include "preferences.h"

//**************************************************************************
// Color stuffs
//**************************************************************************

color colors[NB_COLOR] = {
	{ 0xFFFFFF00, "White"},
	{ 0x00000000, "Black"},
	{ 0xFFFF0000, "Yellow"},
	{ 0xFF000000, "Red"},
};

std::string getColorFromHex(unsigned int hex, color * pColor, int max)
{
	std::string dest;
	for (int i = 0; i < max; i++) {
		if (pColor[i].hex == hex) {
			dest = pColor[i].string;
			break;
		}
	}
	if (dest.empty()) {
		// set it to the hexadecimal value of the color
		char buf[16];		
		sprintf(buf, "%08x", hex);
		dest = buf;
		
	}
	return dest;
}

unsigned int getColorFromString(std::string str, color * pColor, int max)
{
	unsigned int dest = 0;
	for (int i = 0; i < max; i++) {
		if (str == pColor[i].string) {
			dest = pColor[i].hex;
			break;
		}
	}
	return dest;
}

int getColorIndex(unsigned int hex, color * pColor, int max)
{
	for (int i = 0; i < max; i++) {
		if (pColor[i].hex == hex) {
			return i;
		}
	}
	return -1;
}

//**************************************************************************
// Lang and cp
//**************************************************************************

LANG languages[LANGUAGE_SIZE] = {
	{ "Abkhazian", "ab", "abk"},
	{ "Afar", "aa", "aar"},
	{ "Afrikaans", "af", "afr"},
	{ "Albanian", "sq", "sqi"},
	{ "Amharic", "am", "amh"},
	{ "Arabic", "ar", "ara"},
	{ "Aragonese", "an", "arg"},
	{ "Armenian", "hy", "hye"},
	{ "Assamese", "as", "asm"},
	{ "Avestan", "ae", "ave"},
	{ "Aymara", "ay", "aym"},
	{ "Azerbaijani", "az", "aze"},
	{ "Bashkir", "ba", "bak"},
	{ "Basque", "eu", "baq"},
	{ "Belarusian", "be", "bel"},
	{ "Bengali", "bn", "ben"},
	{ "Bihari", "bh", "bih"},
	{ "Bislama", "bi", "bis"},
	{ "Bosnian", "bs", "bos"},
	{ "Breton", "br", "bre"},
	{ "Bulgarian", "bg", "bul"},
	{ "Burmese", "my", "mya"},
	{ "Cambodian", "km", "khm"},
	{ "Catalan", "ca", "cat"},
	{ "Chinese", "zh", "chi"},
	{ "Corsican", "co", "cos"},
	{ "Ceske", "cs", "cze"},
	{ "Dansk", "da", "dan"},
	{ "Deutsch", "de", "ger"},
	{ "English", "en", "eng"},
	{ "Esperanto", "eo", "epo"},
	{ "Español", "es", "spa"},
	{ "Estonian", "et", "est"},
	{ "Finnish", "fi", "fin"},
	{ "Fiji", "fj", "fij"},
	{ "Faroese", "fo", "fao"},
	{ "Français", "fr", "fre"},
	{ "Frisian", "fy", "fry"},
	{ "Galician", "gl", "glg"},
	{ "Georgian", "ka", "geo"},
	{ "Greek", "el", "gre"},
	{ "Greenlandic", "kl", "kal"},
	{ "Guarani", "gn", "grn"},
	{ "Gujarati", "gu", "guj"},
	{ "Hausa", "ha", "hau"},
	{ "Hebrew", "he", "heb"},
	{ "Hindi", "hi", "hin"},
	{ "Hrvatski", "hr", "scr"},
	{ "Indonesian", "id", "ind"},
	{ "Interlingue", "ie", "ile"},
	{ "Inupiak", "ik", "ipk"},
	{ "Irish", "ga", "gle"},
	{ "Islenska", "is", "ice"},
	{ "Italiano", "it", "ita"},
	{ "Inuktitut", "iu", "iku"},
	{ "Japanese", "ja", "jpn"},
	{ "Javanese", "jw", "jav"},
	{ "Kannada", "kn", "kan"},
	{ "Kashmiri", "ks", "kas"},
	{ "Kazakh", "kk", "kaz"},
	{ "Korean", "ko", "kor"},
	{ "Kurdish", "ku", "kur"},
	{ "Kinyarwanda", "rw", "kin"},
	{ "Kirghiz", "ky", "kir"},
	{ "Kirundi", "rn", "run"},
	{ "Latin", "la", "lat"},
	{ "Lingala", "ln", "lin"},
	{ "Laothian", "lo", "lao"},
	{ "Lithuanian", "lt", "lit"},
	{ "Latvian", "lv", "lav"},
	{ "Macedonian", "mk", "mac"},
	{ "Magyar", "hu", "hun"},
	{ "Malagasy", "mg", "mlg"},
	{ "Malay", "ms", "may"},
	{ "Malayalam", "ml", "mal"},
	{ "Maltese", "mt", "mlt"},
	{ "Maori", "mi", "mao"},
	{ "Marathi", "mr", "mar"},
	{ "Moldavian", "mo", "mol"},
	{ "Mongolian", "mn", "mon"},
	{ "Nauru", "na", "nau"},
	{ "Nederlands", "nl", "dut"},
	{ "Nepali", "ne", "nep"},
	{ "Norsk", "no", "nno"},
	{ "Occitan", "oc", "oci"},
	{ "Oriya", "or", "ori"},
	{ "Oromo", "om", "orm"},
	{ "Pashto", "ps", "pus"},
	{ "Persian", "fa", "per"},
	{ "Polish", "pl", "pol"},
	{ "Portugues", "pt", "por"},
	{ "Panjabi", "pa", "pan"},
	{ "Quechua", "qu", "que"},
	{ "Romanian", "ro", "rum"},
	{ "Russian", "ru", "rus"},
	{ "Sangho", "sg", "sag"},
	{ "Samoan", "sm", "smo"},
	{ "Sanskrit", "sa", "san"},
	{ "Scots", "gd", "sco"},
	{ "Serbian", "sr", "scc"},
	{ "Shona", "sn", "sna"},
	{ "Sinhalese", "si", "sin"},
	{ "Sindhi", "sd", "snd"},
	{ "Slovak", "sk", "slo"},
	{ "Slovenian", "sl", "slv"},
	{ "Somali", "so", "som"},
	{ "Sundanese", "su", "sun"},
	{ "Swahili", "sw", "swa"},
	{ "Swedish", "sv", "swe"},
	{ "Tagalog", "tl", "tgl"},
	{ "Tajik", "tg", "tgk"},
	{ "Tamil", "ta", "tam"},
	{ "Tatar", "tt", "tat"},
	{ "Telugu", "te", "tel"},
	{ "Thai", "th", "tha"},
	{ "Tibetan", "bo", "tib"},
	{ "Tigrinya", "ti", "tir"},
	{ "Tonga", "to", "ton"},
	{ "Tsonga", "ts", "tso"},
	{ "Turkish", "tr", "tur"},
	{ "Turkmen", "tk", "tuk"},
	{ "Twi", "tw", "twi"},
	{ "Uighur", "ug", "uig"},
	{ "Ukrainian", "uk", "ukr"},
	{ "Urdu", "ur", "urd"},
	{ "Uzbek", "uz", "uzb"},
	{ "Vietnamese", "ui", "vie"},
	{ "Volapuk", "vo", "Vol"},
	{ "Welsh", "cy", "wel"},
	{ "Wolof", "wo", "wol"},
	{ "Xhosa", "xh", "xho"},
	{ "Yiddish", "yi", "yid"},
	{ "Yoruba", "yo", "yor"},
	{ "Zhuang", "za", "zha"},
	{ "Zulu", "zu", "zul"}
};

CP codepages[CODEPAGE_SIZE] = {
	{ "utf-8", "UTF-8"},
	{ "ISO-8859-1", "Western European"},
	{ "ISO-8859-2", "Eastern European"},
	{ "ISO-8859-3", "South European"},
	{ "ISO-8859-4", "North European"},
	{ "ISO-8859-5", "Cyrillic alphabets"},
	{ "ISO-8859-6", "Arabic"},
	{ "ISO-8859-7", "Greek"},
	{ "ISO-8859-8", "Hebrew"},
	{ "ISO-8859-9", "Turkish"},
	{ "ISO-8859-10", "Nordic"},
	{ "ISO-8859-11", "Thai"},
	{ "ISO-8859-13", "Baltic and Polish"},
	{ "ISO-8859-14", "Celtic"},
	{ "Windows-1250", "Central/Eastern European"},
	{ "Windows-1251", "Cyrillic alphabets"},
	{ "Windows-1252", "Western European 2"},
	{ "Windows-1253", "Greek 2"},
	{ "Windows-1254", "Turkish 2"},
	{ "Windows-1255", "Hebrew 2"},
	{ "Windows-1256", "Arabic 2"},
	{ "Windows-1257", "Baltic 2"},
	{ "Windows-1258", "Vietnamese"},
	{ "shift_jis", "Japanese (Shift JIS)"},
	{ "gb2312", "Chinese Simplified (GB2312)"},
	{ "big5", "Chinese Traditional (Big5)"},
	{ "cp949", "Korean (CP949)"}
};


int GetAudioLangIndex()
{
	for (int i = 0; i < LANGUAGE_SIZE; i++)
		if (strcmp(XMPlayerCfg.alang, languages[i].abbrev2.c_str()) == 0)
			return i;
	return 0;
}

int GetCodepageIndex()
{
	for (int i = 0; i < CODEPAGE_SIZE; i++)
		if (strcmp(XMPlayerCfg.subcp, codepages[i].cpname.c_str()) == 0)
			return i;
	return 0;
}

int GetSubLangIndex()
{
	for (int i = 0; i < LANGUAGE_SIZE; i++)
		if (strcmp(XMPlayerCfg.sublang, languages[i].abbrev.c_str()) == 0)
			return i;
	return 0;
}
