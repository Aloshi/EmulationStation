#pragma once

#include "Scraper.h"

void thegamesdb_generate_scraper_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests, 
	std::vector<ScraperSearchResult>& results);

void thegamesdb_process_httpreq(const std::unique_ptr<HttpReq>& req, std::vector<ScraperSearchResult>& results);
