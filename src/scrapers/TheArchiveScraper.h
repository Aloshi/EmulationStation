#pragma once

#include "Scraper.h"
#include "../HttpReq.h"

class TheArchiveHandle : public ScraperSearchHandle
{
public:
	TheArchiveHandle(const ScraperSearchParams& params, const std::string& url);

	void update() override;

private:
	std::unique_ptr<HttpReq> mReq;
	ScraperSearchParams mParams;
};

class TheArchiveScraper : public Scraper
{
public:
	std::unique_ptr<ScraperSearchHandle> getResultsAsync(const ScraperSearchParams& params) override;

	const char* getName();
};
