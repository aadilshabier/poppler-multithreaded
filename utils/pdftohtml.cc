//========================================================================
//
// pdftohtml.cc
//
//
// Copyright 1999-2000 G. Ovtcharov
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2007-2008, 2010, 2012, 2015-2020, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2010 Mike Slegeir <tehpola@yahoo.com>
// Copyright (C) 2010, 2013 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
// Copyright (C) 2010 OSSD CDAC Mumbai by Leena Chourey (leenac@cdacmumbai.in) and Onkar Potdar (onkar@cdacmumbai.in)
// Copyright (C) 2011 Steven Murdoch <Steven.Murdoch@cl.cam.ac.uk>
// Copyright (C) 2012 Igor Slepchin <igor.redhat@gmail.com>
// Copyright (C) 2012 Ihar Filipau <thephilips@gmail.com>
// Copyright (C) 2012 Luis Parravicini <lparravi@gmail.com>
// Copyright (C) 2014 Pino Toscano <pino@kde.org>
// Copyright (C) 2015 William Bader <williambader@hotmail.com>
// Copyright (C) 2017, 2021 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018 Thibaut Brard <thibaut.brard@gmail.com>
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019, 2021 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2021 Hubert Figuiere <hub@figuiere.net>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include "config.h"
#include <poppler-config.h>
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#ifdef HAVE_DIRENT_H
#    include <dirent.h>
#endif
#include <ctime>
#include "parseargs.h"
#include "goo/GooString.h"
#include "goo/gbase64.h"
#include "goo/gbasename.h"
#include "goo/gmem.h"
#include "Object.h"
#include "Stream.h"
#include "Array.h"
#include "Dict.h"
#include "XRef.h"
#include "Catalog.h"
#include "Page.h"
#include "Outline.h"
#include "PDFDoc.h"
#include "PDFDocFactory.h"
#include "HtmlOutputDev.h"
#include "GlobalParams.h"
#include "PDFDocEncoding.h"
#include "Error.h"
#include "DateInfo.h"
#include "goo/gfile.h"
#include "Win32Console.h"

#include <thread>
#include <future>
#include <vector>
#include <cmath>

#include "nlohmann/json.hpp"
#include "AWSHelper.h"

#include <aws/core/utils/UUID.h>

using json = nlohmann::json;

static int firstPage = 1;
static int lastPage = 0;
static bool rawOrder = true;
bool printCommands = true;
static bool printHelp = false;
bool printHtml = false;
static const bool complexMode = true;
static const bool singleHtml = false;
bool dataUrls = false;
bool ignore = false;
static char extension[5] = "png";
static double scale = 1.5;
// static const bool noframes = true;
bool stout = false;
static bool jsonFlag = false;
bool noRoundedCoordinates = false;
static bool errQuiet = false;
static bool noDrm = false;
double wordBreakThreshold = 10; // 10%, below converted into a coefficient - 0.1
static int jobs = 0;

bool showHidden = false;
// static const bool noMerge = true;
bool fontFullName = false;
static char ownerPassword[33] = "";
static char userPassword[33] = "";
static bool printVersion = false;

static GooString *getInfoString(Dict *infoDict, const char *key);
static GooString *getInfoDate(Dict *infoDict, const char *key);

static char textEncName[128] = "";

char bucketNameArg[64] = "";

static const ArgDesc argDesc[] = { { "-f", argInt, &firstPage, 0, "first page to convert" },
                                   { "-l", argInt, &lastPage, 0, "last page to convert" },
                                   { "-j", argInt, &jobs, 0, "number of jobs (defaults to all cores)" },
                                   /*{"-raw",    argFlag,     &rawOrder,      0,
                                     "keep strings in content stream order"},*/
                                   { "-q", argFlag, &errQuiet, 0, "don't print any messages or errors" },
                                   { "-h", argFlag, &printHelp, 0, "print usage information" },
                                   { "-?", argFlag, &printHelp, 0, "print usage information" },
                                   { "-help", argFlag, &printHelp, 0, "print usage information" },
                                   { "--help", argFlag, &printHelp, 0, "print usage information" },
                                   { "-p", argFlag, &printHtml, 0, "exchange .pdf links by .html" },
                                   { "-i", argFlag, &ignore, 0, "ignore images" },
                                   { "-stdout", argFlag, &stout, 0, "use standard output" },
                                   { "-zoom", argFP, &scale, 0, "zoom the pdf document (default 1.5)" },
                                   { "-json", argFlag, &jsonFlag, 0, "output for JSON post-processing" },
                                   { "-noroundcoord", argFlag, &noRoundedCoordinates, 0, "do not round coordinates (with XML output only)" },
                                   { "-hidden", argFlag, &showHidden, 0, "output hidden text" },
                                   { "-enc", argString, textEncName, sizeof(textEncName), "output text encoding name" },
                                   { "-fmt", argString, extension, sizeof(extension), "image file format for Splash output (png or jpg)" },
                                   { "-v", argFlag, &printVersion, 0, "print copyright and version info" },
                                   { "-opw", argString, ownerPassword, sizeof(ownerPassword), "owner password (for encrypted files)" },
                                   { "-upw", argString, userPassword, sizeof(userPassword), "user password (for encrypted files)" },
                                   { "-nodrm", argFlag, &noDrm, 0, "override document DRM settings" },
                                   { "-wbt", argFP, &wordBreakThreshold, 0, "word break threshold (default 10 percent)" },
                                   { "-fontfullname", argFlag, &fontFullName, 0, "outputs font full name" },
								   { "-bucket", argString, bucketNameArg, sizeof(bucketNameArg), "name of AWS S3 bucket"},
                                   {} };

int main(int argc, char *argv[])
{
    std::unique_ptr<PDFDoc> doc;
    GooString *fileName = nullptr;
    GooString *docTitle = nullptr;
    GooString *author = nullptr, *keywords = nullptr, *subject = nullptr, *date = nullptr;
    GooString *htmlFileName = nullptr;
    bool doOutline;
    bool ok;
    std::optional<GooString> ownerPW, userPW;
    Object info;
    int exit_status = EXIT_FAILURE;

    Win32Console win32Console(&argc, &argv);
    // parse args
    ok = parseArgs(argDesc, &argc, argv);
    if (!ok || argc < 2 || argc > 3 || printHelp || printVersion) {
        fprintf(stderr, "pdftohtml version %s\n", PACKAGE_VERSION);
        fprintf(stderr, "%s\n", popplerCopyright);
        fprintf(stderr, "%s\n", "Copyright 1999-2003 Gueorgui Ovtcharov and Rainer Dorsch");
        fprintf(stderr, "%s\n\n", xpdfCopyright);
        if (!printVersion) {
            printUsage("pdftohtml -json -bucket <BUCKETNAME>", "<PDF-file> [<OUTPUT-folder>]", argDesc);
        }
        exit(printHelp || printVersion ? 0 : 1);
    }

	if (!strncmp("", bucketNameArg, sizeof(bucketNameArg))) {
		fprintf(stderr, "ERROR: Please set a bucket name using the -bucket argument!\n");
		goto error;
	}

	if (!jsonFlag) {
		fprintf(stderr, "ERROR: This is a modified version of pdftohtml which is only meant to be used for JSON output!\n");
		goto error;
	}

	if (auto& aws = AWSHelper::GetInstance(); !aws.IsInit()) {
		goto error;
	}

    // init error file
    // errorInit();

    // read config file
    globalParams = std::make_unique<GlobalParams>();

    if (errQuiet) {
        globalParams->setErrQuiet(errQuiet);
        printCommands = false; // I'm not 100% what is the difference between them
    }

    if (textEncName[0]) {
        globalParams->setTextEncoding(textEncName);
        if (!globalParams->getTextEncoding()) {
            goto error;
        }
    }

    // convert from user-friendly percents into a coefficient
    wordBreakThreshold /= 100.0;

    // open PDF file
    if (ownerPassword[0]) {
        ownerPW = GooString(ownerPassword);
    }
    if (userPassword[0]) {
        userPW = GooString(userPassword);
    }

    fileName = new GooString(argv[1]);

    if (fileName->cmp("-") == 0) {
        delete fileName;
        fileName = new GooString("fd://0");
    }

    doc = PDFDocFactory().createPDFDoc(*fileName, ownerPW, userPW);

    if (!doc->isOk()) {
        goto error;
    }

    // check for copy permission
    if (!doc->okToCopy()) {
        if (!noDrm) {
            error(errNotAllowed, -1, "Copying of text from this document is not allowed.");
            goto error;
        }
        fprintf(stderr, "Document has copy-protection bit set.\n");
    }

    // construct text file name
    if (argc == 3) {
        htmlFileName = new GooString(argv[2]);
    } else if (fileName->cmp("fd://0") == 0) {
        error(errCommandLine, -1, "You have to provide an output folder when reading from stdin.");
        goto error;
    } else {
        const char *p = fileName->c_str() + fileName->getLength() - 4;
        if (!strcmp(p, ".pdf") || !strcmp(p, ".PDF")) {
            htmlFileName = new GooString(fileName->c_str(), fileName->getLength() - 4);
        } else {
            htmlFileName = fileName->copy();
        }
        //   htmlFileName->append(".html");
    }

    if (scale > 3.0) {
        scale = 3.0;
    }
    if (scale < 0.5) {
        scale = 0.5;
    }

	if (complexMode) {
        stout = false;
    }

    // get page range
    if (firstPage < 1) {
        firstPage = 1;
    }
    if (lastPage < 1 || lastPage > doc->getNumPages()) {
        lastPage = doc->getNumPages();
    }
    if (lastPage < firstPage) {
        error(errCommandLine, -1, "Wrong page range given: the first page ({0:d}) can not be after the last page ({1:d}).", firstPage, lastPage);
        goto error;
    }

	if (jobs == 0) {
		jobs = std::thread::hardware_concurrency();
	}

    info = doc->getDocInfo();
    if (info.isDict()) {
        docTitle = getInfoString(info.getDict(), "Title");
        author = getInfoString(info.getDict(), "Author");
        keywords = getInfoString(info.getDict(), "Keywords");
        subject = getInfoString(info.getDict(), "Subject");
        date = getInfoDate(info.getDict(), "ModDate");
        if (!date) {
            date = getInfoDate(info.getDict(), "CreationDate");
        }
    }
    if (!docTitle) {
        docTitle = new GooString(htmlFileName);
    }

    if (!singleHtml) {
        rawOrder = complexMode; // todo: figure out what exactly rawOrder do :)
    } else {
        rawOrder = singleHtml;
    }

    doOutline = doc->getOutline()->getItems() != nullptr;

	// Append random string to end of filename
	htmlFileName->append(" - ");
	htmlFileName->append(Aws::Utils::UUID::RandomUUID());
	// scope to allow goto to work
	{
		const auto f = [=](PDFDoc* doc, int i, int startPage, int endPage, std::promise<Contents> promise) {
			auto out = HtmlOutputDev(doc->getCatalog(), htmlFileName->c_str(), docTitle->c_str(), author ? author->c_str() : nullptr, keywords ? keywords->c_str() : nullptr, subject ? subject->c_str() : nullptr, date ? date->c_str() : nullptr, rawOrder, firstPage, doOutline);
			doc->displayPages(&out, startPage, endPage, 72 * scale, 72 * scale, 0, true, false, false);
			out.dumpDocOutline(doc);
			auto r = out.extractContents();
			promise.set_value(r);
		};

		std::vector<std::thread> threads(jobs);
		std::vector<std::future<Contents>> futures(jobs);
		const int d = ceil(static_cast<float>(lastPage-firstPage+1) / static_cast<float>(jobs));
		for (int i=0; i<jobs; i++) {
			int startPage = firstPage + i*d;
			int endPage = std::min(lastPage, firstPage-1 + (i+1)*d);
			if (printCommands)
				printf("Thread %d(Pages %d to %d)\n", i, startPage, endPage);
			std::promise<Contents> promise;
		    futures[i] = promise.get_future();
			threads[i] = std::thread(f, doc.get(), i, startPage, endPage, std::move(promise));
		}
		json arr = json::array();
		for (int i=0; i<jobs; i++) {
		    threads[i].join();
			auto r = json(futures[i].get());
			for (auto & x : r)
				arr.push_back(std::move(x));
		}
		std::string jsonFileName = htmlFileName->toStr() + "/output.json";
	    std::shared_ptr<Aws::IOStream> stream = Aws::MakeShared<Aws::StringStream>(AWSHelper::ALLOC_TAG);
		*stream << arr << std::endl;
		AWSHelper::GetInstance().PutObject(jsonFileName, stream);
	}

    exit_status = EXIT_SUCCESS;

    // clean up
error:
    delete fileName;

	delete docTitle;

    if (author) {
        delete author;
    }
    if (keywords) {
        delete keywords;
    }
    if (subject) {
        delete subject;
    }
    if (date) {
        delete date;
    }

    if (htmlFileName) {
        delete htmlFileName;
    }

    return exit_status;
}

static GooString *getInfoString(Dict *infoDict, const char *key)
{
    Object obj;
    // Raw value as read from PDF (may be in pdfDocEncoding or UCS2)
    const GooString *rawString;
    // Value converted to unicode
    Unicode *unicodeString;
    int unicodeLength;
    // Value HTML escaped and converted to desired encoding
    GooString *encodedString = nullptr;
    // Is rawString UCS2 (as opposed to pdfDocEncoding)
    bool isUnicode;

    obj = infoDict->lookup(key);
    if (obj.isString()) {
        rawString = obj.getString();

        // Convert rawString to unicode
        if (rawString->hasUnicodeMarker()) {
            isUnicode = true;
            unicodeLength = (obj.getString()->getLength() - 2) / 2;
        } else {
            isUnicode = false;
            unicodeLength = obj.getString()->getLength();
        }
        unicodeString = new Unicode[unicodeLength];

        for (int i = 0; i < unicodeLength; i++) {
            if (isUnicode) {
                unicodeString[i] = ((rawString->getChar((i + 1) * 2) & 0xff) << 8) | (rawString->getChar(((i + 1) * 2) + 1) & 0xff);
            } else {
                unicodeString[i] = pdfDocEncoding[rawString->getChar(i) & 0xff];
            }
        }

        // HTML escape and encode unicode
        encodedString = HtmlFont::HtmlFilter(unicodeString, unicodeLength);
        delete[] unicodeString;
    }

    return encodedString;
}

static GooString *getInfoDate(Dict *infoDict, const char *key)
{
    Object obj;
    int year, mon, day, hour, min, sec, tz_hour, tz_minute;
    char tz;
    struct tm tmStruct;
    GooString *result = nullptr;
    char buf[256];

    obj = infoDict->lookup(key);
    if (obj.isString()) {
        const GooString *s = obj.getString();
        // TODO do something with the timezone info
        if (parseDateString(s, &year, &mon, &day, &hour, &min, &sec, &tz, &tz_hour, &tz_minute)) {
            tmStruct.tm_year = year - 1900;
            tmStruct.tm_mon = mon - 1;
            tmStruct.tm_mday = day;
            tmStruct.tm_hour = hour;
            tmStruct.tm_min = min;
            tmStruct.tm_sec = sec;
            tmStruct.tm_wday = -1;
            tmStruct.tm_yday = -1;
            tmStruct.tm_isdst = -1;
            mktime(&tmStruct); // compute the tm_wday and tm_yday fields
            if (strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S+00:00", &tmStruct)) {
                result = new GooString(buf);
            } else {
                result = new GooString(s);
            }
        } else {
            result = new GooString(s);
        }
    }
    return result;
}
