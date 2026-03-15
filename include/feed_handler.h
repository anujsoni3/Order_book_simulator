#pragma once

#include "matching_engine.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace order_book {

class FeedHandler {
public:
    static std::vector<Trade> playback_csv(MatchingEngine& engine, const std::string& filename) {
        std::ifstream file(filename);
        std::string line;
        std::vector<Trade> total_trades;

        if (!file.is_open()) return total_trades;

        // Skip header if exists
        std::getline(file, line);

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string item;
            std::vector<std::string> row;

            while (std::getline(ss, item, ',')) {
                row.push_back(item);
            }

            if (row.size() < 6) continue;

            NewOrder order;
            order.id = std::stoull(row[0]);
            order.side = (row[1] == "BUY" ? Side::BUY : Side::SELL);
            
            if (row[2] == "LIMIT") order.type = OrderType::LIMIT;
            else if (row[2] == "MARKET") order.type = OrderType::MARKET;
            else if (row[2] == "IOC") order.type = OrderType::IOC;
            else if (row[2] == "FOK") order.type = OrderType::FOK;
            else if (row[2] == "POST_ONLY") order.type = OrderType::POST_ONLY;

            order.price = std::stoll(row[3]);
            order.quantity = std::stoll(row[4]);
            order.timestamp = std::stoull(row[5]);
            order.flags = 0;

            auto trades = engine.add_order(order);
            total_trades.insert(total_trades.end(), trades.begin(), trades.end());
        }

        return total_trades;
    }
};

}
