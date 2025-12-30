#pragma once

#include "api.h"
#include <queue>


namespace Autonomous {
        struct Action {
                float time = 0;
                bool blocking = true;

                virtual void run_tick() = 0;
        };

        struct Align : public Action {
                struct Config {
                        float K;
                        float B;
                        float A;
                        float angle;
                        float epsilon;
                        float timeout;
                };

                Config c;

                Align(Config c);
                void run_tick() override;
        };

        struct Travel : public Action {
                struct Config {
                        float K;
                        float B;
                        float A;
                        float dist;
                        float epsilon;
                        float timeout;
                };

                Config c;

                Travel(Config c);
                void run_tick() override;
        };

        typedef std::pair<std::string, std::vector<std::unique_ptr<Action>>> Routine;

        extern std::shared_ptr<Routine> active_routine;
        // <Name (of routine), Routine>
        extern std::vector<std::shared_ptr<Routine>> routines;
        extern std::queue<std::unique_ptr<Action>> actions_queue;
        extern std::vector<std::unique_ptr<Action>> actions_running;

        void select_routine(int index);
        void initialize_actions_queue(std::shared_ptr<Routine> routine);
        void run_action(std::unique_ptr<Action>* action);
}