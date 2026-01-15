#include "orchestrator.hpp"


std::vector<Attempt*> Orchestrator::SyncAttempt(Attempt& attempt)
{
    std::vector<Attempt*> all_completed_attempts{};
    while (!attempt.IsDone()) {
        std::vector<Attempt*> completed_attempts{ExecOnce()};
        for (Attempt* completed_attempt : completed_attempts) {
            if (!completed_attempt->IsDone()) { continue; }
            auto it = std::ranges::find(all_completed_attempts, completed_attempt);
            if (it == all_completed_attempts.end()) {
                all_completed_attempts.push_back(completed_attempt);
            }
        }
    }
    return all_completed_attempts;
}
