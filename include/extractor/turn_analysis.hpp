#ifndef OSRM_EXTRACTOR_TURN_ANALYSIS
#define OSRM_EXTRACTOR_TURN_ANALYSIS

#include "engine/guidance/turn_classification.hpp"
#include "engine/guidance/guidance_toolkit.hpp"

#include "extractor/restriction_map.hpp"
#include "extractor/compressed_edge_container.hpp"

#include <unordered_set>

namespace osrm
{
namespace extractor
{

struct TurnCandidate
{
    EdgeID eid;   // the id of the arc
    bool valid;   // a turn may be relevant to good instructions, even if we cannot take the road
    double angle; // the approximated angle of the turn
    engine::guidance::TurnInstruction instruction; // a proposed instruction
    double confidence;                             // how close to the border is the turn?

    std::string toString() const
    {
        std::string result = "[turn] ";
        result += std::to_string(eid);
        result += " valid: ";
        result += std::to_string(valid);
        result += " angle: ";
        result += std::to_string(angle);
        result += " instruction: ";
        result += std::to_string(static_cast<std::int32_t>(instruction.type)) + " " +
                  std::to_string(static_cast<std::int32_t>(instruction.direction_modifier));
        result += " confidence: ";
        result += std::to_string(confidence);
        return result;
    }
};
namespace turn_analysis
{

// the entry into the turn analysis
std::vector<TurnCandidate>
getTurns(const NodeID from_node,
         const EdgeID via_eid,
         const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph,
         const std::vector<QueryNode> &node_info_list,
         const std::shared_ptr<RestrictionMap const> restriction_map,
         const std::unordered_set<NodeID> &barrier_nodes,
         const CompressedEdgeContainer &compressed_edge_container);

namespace detail
{

// Check for restrictions/barriers and generate a list of valid and invalid turns present at the
// node reached
// from `from_node` via `via_eid`
std::vector<TurnCandidate>
getTurnCandidates(const NodeID from_node,
                  const EdgeID via_eid,
                  const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph,
                  const std::vector<QueryNode> &node_info_list,
                  const std::shared_ptr<RestrictionMap const> restriction_map,
                  const std::unordered_set<NodeID> &barrier_nodes,
                  const CompressedEdgeContainer &compressed_edge_container);

// merge segregated roads to omit invalid turns in favor of treating segregated roads as one
std::vector<TurnCandidate>
mergeSegregatedRoads(const NodeID from_node,
                     const EdgeID via_eid,
                     std::vector<TurnCandidate> turn_candidates,
                     const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph);

// handle roundabouts
// TODO distinguish roundabouts and rotaries
// TODO handle bike/walk cases that allow crossing a roundabout!
std::vector<TurnCandidate>
handleRoundabouts(const NodeID from,
                  const EdgeID via_edge,
                  const bool on_roundabout,
                  const bool can_enter_roundabout,
                  const bool can_exit_roundabout,
                  std::vector<TurnCandidate> turn_candidates,
                  const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph);

bool isBasicJunction(const NodeID from,
                     const EdgeID via_edge,
                     const std::vector<TurnCandidate> &turn_candidates,
                     const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph);

bool isMotorwayJunction(const NodeID from,
                        const EdgeID via_edge,
                        const std::vector<TurnCandidate> &turn_candidates,
                        const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph);

// Decide whether a turn is a turn or a ramp access
engine::guidance::TurnType
turnOrRamp(const NodeID from,
           const EdgeID via_edge,
           const TurnCandidate &candidate,
           const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph);

// Get the Instruction for an obvious turn
engine::guidance::TurnInstruction
getInstructionForObvious(const NodeID from,
                         const EdgeID via_edge,
                         const TurnCandidate &candidate,
                         const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph);

engine::guidance::TurnInstruction
noTurnOrNewName(const NodeID from,
                const EdgeID via_edge,
                const TurnCandidate &candidate,
                const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph);

// handle basic intersections
std::vector<TurnCandidate>
handleOneWayTurn(const NodeID from,
                 const EdgeID via_edge,
                 std::vector<TurnCandidate> turn_candidates,
                 const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph);

std::vector<TurnCandidate>
handleTwoWayTurn(const NodeID from,
                 const EdgeID via_edge,
                 std::vector<TurnCandidate> turn_candidates,
                 const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph);

std::vector<TurnCandidate>
handleThreeWayTurn(const NodeID from,
                   const EdgeID via_edge,
                   std::vector<TurnCandidate> turn_candidates,
                   const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph);

std::vector<TurnCandidate>
handleFourWayTurn(const NodeID from,
                  const EdgeID via_edge,
                  std::vector<TurnCandidate> turn_candidates,
                  const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph);

std::vector<TurnCandidate>
handleComplexTurn(const NodeID from,
                  const EdgeID via_edge,
                  std::vector<TurnCandidate> turn_candidates,
                  const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph);

std::vector<TurnCandidate>
handleMotorwayJunction(const NodeID from,
                       const EdgeID via_edge,
                       std::vector<TurnCandidate> turn_candidates,
                       const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph);

std::vector<TurnCandidate>
setTurnTypes(const NodeID from,
             const EdgeID via_edge,
             std::vector<TurnCandidate> turn_candidates,
             const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph);

std::vector<TurnCandidate>
optimizeRamps(const EdgeID via_edge,
              std::vector<TurnCandidate> turn_candidates,
              const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph);

std::vector<TurnCandidate>
optimizeCandidates(const EdgeID via_eid,
                   std::vector<TurnCandidate> turn_candidates,
                   const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph,
                   const std::vector<QueryNode> &node_info_list);

bool isObviousChoice(const EdgeID via_eid,
                     const std::size_t turn_index,
                     const std::vector<TurnCandidate> &turn_candidates,
                     const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph);

std::vector<TurnCandidate>
suppressTurns(const EdgeID via_eid,
              std::vector<TurnCandidate> turn_candidates,
              const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph);

// node_u -- (edge_1) --> node_v -- (edge_2) --> node_w
engine::guidance::TurnInstruction
AnalyzeTurn(const NodeID node_u,
            const EdgeID edge1,
            const NodeID node_v,
            const EdgeID edge2,
            const NodeID node_w,
            const double angle,
            const std::shared_ptr<const util::NodeBasedDynamicGraph> node_based_graph);
} // namespace detail
} // namespace turn_analysis
} // namespace extractor
} // namespace osrm

#endif // OSRM_EXTRACTOR_TURN_ANALYSIS
