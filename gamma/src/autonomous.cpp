#include "autonomous.hpp"
#include "properties.hpp"


namespace Autonomous {
        std::shared_ptr<Routine> active_routine;
        std::vector<std::shared_ptr<Routine>> routines;

        Align::Align(Align::Config c) {this -> c = c;}
        void Align::run_tick() {
                time += Properties::TICK_DELAY_MSEC/1000.0;
        }

        void select_routine(int index) {
                active_routine = routines.at(index);
        }
}