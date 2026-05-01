#include <cmath>
#include <cstdint>
#include <rclcpp/rclcpp.hpp>

template <typename VehicleType>
class RosVehicleAdapter
{
public:
  float handleSpeed(float speed) { return static_cast<VehicleType *>(this)->handleSpeed(speed); }
  int handleSteering(float steering_angle)
  {
    return static_cast<VehicleType *>(this)->handleSteering(steering_angle);
  }
  std::uint8_t handleGear(std::uint8_t gear)
  {
    return static_cast<VehicleType *>(this)->handleGear(gear);
  }
  std::uint8_t handleIndicators(std::uint8_t indicator_state)
  {
    return static_cast<VehicleType *>(this)->handleIndicators(indicator_state);
  }
};

/*For further info regarding conversions and expected input/output consult the
 * repo that holds the low level controller*/

class Twizy : public RosVehicleAdapter<Twizy>
{
public:
  std::uint8_t autoware2VehicleIndicator(const std::uint8_t indicator) const noexcept
  {
    if (indicator == 2) {
      return 1;
    } else if (indicator == 3) {
      return 2;
    } else {
      return 0;
    }
  }

  std::uint8_t autoware2VehicleGear(const std::uint8_t gear) const noexcept
  {
    if (gear == 20) {
      return 2;
    } else if (gear == 2) {
      return 1;
    } else {
      return 0;
    }
  }

  float handleSpeed(const float speed) const noexcept { return speed; }
  int handleSteering(const float steering_angle) noexcept
  {
     RCLCPP_INFO_STREAM( rclcpp::get_logger("steering_device_monitor"), "STEERING handleSteering " << steering_angle);
    return static_cast<int>(rad2deg(steering_angle / tire_to_wheel_ratio_));
  }

  std::uint8_t handleGear(const std::uint8_t gear) const noexcept
  {
    return autoware2VehicleGear(gear);
  }
  std::uint8_t handleIndicators(std::uint8_t indicator_state) const noexcept
  {
    return autoware2VehicleIndicator(indicator_state);
  }

private:
  // Tire angle to steering wheel angle ratio
  static constexpr float tire_to_wheel_ratio_ = 40. / 520.;

  static_assert(tire_to_wheel_ratio_ > 0.001, "tire_to_wheel_ratio_ must not be zero");
  float rad2deg(float angle_in_rads) { 
    RCLCPP_INFO_STREAM( rclcpp::get_logger("steering_device_monitor"), "STEERING rad2deg " << angle_in_rads);
    return angle_in_rads * 180.f / M_PI; 
  }
};

class Citaro : public RosVehicleAdapter<Citaro>
{
public:
  std::uint8_t autoware2VehicleIndicator(const std::uint8_t indicator) const noexcept
  {
    if (indicator == 2) {
      return 1;
    } else if (indicator == 3) {
      return 2;
    } else {
      return 0;
    }
  }

  std::uint8_t autoware2VehicleGear(const std::uint8_t gear) const noexcept
  {
    if (gear == 20) {
      return 2;
    } else if (gear == 2) {
      return 1;
    } else {
      return 0;
    }
  }

  float handleSpeed(const float speed) const noexcept { return speed; }
  int handleSteering(const float steering_angle) noexcept
  {
    RCLCPP_INFO_STREAM( rclcpp::get_logger("steering_device_monitor"), "STEERING steering_angle " << steering_angle);
    return static_cast<int>(rad2deg(steering_angle / tire_to_wheel_ratio_));
  }

  std::uint8_t handleGear(const std::uint8_t gear) const noexcept
  {
    return autoware2VehicleGear(gear);
  }
  std::uint8_t handleIndicators(std::uint8_t indicator_state) const noexcept
  {
    return autoware2VehicleIndicator(indicator_state);
  }

private:
  // Tire angle to steering wheel angle ratio
  static constexpr float tire_to_wheel_ratio_ = 15.63 / 840.;

  static_assert(tire_to_wheel_ratio_ > 0.001, "tire_to_wheel_ratio_ must not be zero");

  float rad2deg(float angle_in_rads) { return angle_in_rads * 180.f / M_PI; }
};
