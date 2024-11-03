#pragma once
#include "flight_provider.h"
#include "hotel_provider.h"

#include <string>
#include <vector>

class Trip {
 public:
  Trip(HotelProvider &hp, FlightProvider &fp) : hp_(hp), fp_(fp) {}
  Trip(const Trip&);
  Trip(Trip&&);

  ~Trip() {
    for (size_t i = 0; i < flights_.size(); ++i) {
      hp_.Cancel(static_cast<int>(i));
    }
    hp_.counter = 0;

    for (size_t i = 0; i < hotels_.size(); ++i) {
      fp_.Cancel(static_cast<int>(i));
    }
    fp_.counter = 0;
  }

  Trip& operator=(const Trip&);
  Trip& operator=(Trip&&);
  void Cancel();

  std::vector<FlightProvider::BookingId> flights_;
  std::vector<HotelProvider::BookingId> hotels_;
 private:
  HotelProvider &hp_;
  FlightProvider &fp_;
};

class TripManager {
 public:
  using BookingId = std::string;
  struct BookingData {
    std::string city_from;
    std::string city_to;
    std::string date_from;
    std::string date_to;
  };

  Trip Book(const BookingData&) {
    Trip trip(hotel_provider_, flight_provider_);
    {
      FlightProvider::BookingData flight_booking_data;
      trip.flights_.push_back(flight_provider_.Book(flight_booking_data));
    }
    {
      HotelProvider::BookingData hotel_booking_data;
      trip.hotels_.push_back(hotel_provider_.Book(hotel_booking_data));
    }
    {
      FlightProvider::BookingData flight_booking_data;
      trip.flights_.push_back(flight_provider_.Book(flight_booking_data));
    }
    return trip;
  }

  void Cancel(Trip& trip) {
    trip.Cancel();
  }

 private:
  HotelProvider hotel_provider_;
  FlightProvider flight_provider_;
};