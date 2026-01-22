#include "autonomous.hpp"
#include "bot_connections.hpp"
#include "xmath.hpp"


namespace Autonomous {
        void run_odom_tick() {
                int new_odom_cdeg = BotConnections::odom.get_position();
                float heading_rad = XMath::rad(BotConnections::imu.get_heading());
                Vector2 heading_vec {
                        -std::sin(heading_rad),
                        std::cos(heading_rad)
                };

                Properties::odom_position += heading_vec
                        *((new_odom_cdeg - Properties::odom_cdeg) / 100.0)
                        *(Properties::ODOM_DIAMETER_IN/2.0);

                Properties::odom_cdeg = new_odom_cdeg;
        }
}