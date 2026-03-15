#include <gtest/gtest.h>
#include "matching_engine.h"

using namespace order_book;

static NewOrder make_order(OrderId id, Side side, OrderType type, int64_t price, int64_t qty, std::uint64_t ts = 0) {
    return NewOrder{id, side, type, price, qty, ts, 0};
}

TEST(EnhancedMatchingEngineTest, IOC_DoesNotRest) {
    MatchingEngine engine(100);
    // Add a resting sell order
    engine.add_order(make_order(1, Side::SELL, OrderType::LIMIT, 100, 10));
    
    // Add IOC buy order for more than available
    auto trades = engine.add_order(make_order(2, Side::BUY, OrderType::IOC, 100, 50));
    
    EXPECT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].quantity, 10);
    
    // Check that nothing is resting at price 100 for buy side
    EXPECT_TRUE(engine.get_bids().empty());
}

TEST(EnhancedMatchingEngineTest, FOK_FullyFillsOrRejects) {
    MatchingEngine engine(100);
    engine.add_order(make_order(1, Side::SELL, OrderType::LIMIT, 100, 20));
    
    // FOK for 50 should be rejected because only 20 available
    auto trades = engine.add_order(make_order(2, Side::BUY, OrderType::FOK, 100, 50));
    EXPECT_TRUE(trades.empty());
    
    // FOK for 20 should succeed
    trades = engine.add_order(make_order(3, Side::BUY, OrderType::FOK, 100, 20));
    EXPECT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].quantity, 20);
}

TEST(EnhancedMatchingEngineTest, PostOnlyRejectsCrossing) {
    MatchingEngine engine(100);
    engine.add_order(make_order(1, Side::SELL, OrderType::LIMIT, 100, 10));
    
    // Post-only at 100 should be rejected as it crosses best ask
    auto trades = engine.add_order(make_order(2, Side::BUY, OrderType::POST_ONLY, 100, 10));
    EXPECT_TRUE(trades.empty());
    EXPECT_TRUE(engine.get_bids().empty());
}

TEST(EnhancedMatchingEngineTest, MarketOrderSweeps) {
    MatchingEngine engine(100);
    engine.add_order(make_order(1, Side::SELL, OrderType::LIMIT, 100, 10));
    engine.add_order(make_order(2, Side::SELL, 101, 10));
    
    auto trades = engine.add_order(make_order(3, Side::BUY, OrderType::MARKET, 0, 15));
    EXPECT_EQ(trades.size(), 2);
    EXPECT_EQ(trades[0].price, 100);
    EXPECT_EQ(trades[1].price, 101);
}

TEST(EnhancedMatchingEngineTest, TickSizeValidation) {
    MatchingEngine engine(100, 5, 1); // Tick size 5
    
    // Should be rejected
    auto trades = engine.add_order(make_order(1, Side::BUY, OrderType::LIMIT, 102, 10));
    EXPECT_TRUE(trades.empty());
    EXPECT_TRUE(engine.get_bids().empty());
    
    // Should be accepted
    engine.add_order(make_order(2, Side::BUY, OrderType::LIMIT, 105, 10));
    EXPECT_FALSE(engine.get_bids().empty());
}
