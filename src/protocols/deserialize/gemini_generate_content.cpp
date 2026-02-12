#include "../../../interface/protocols/gemini/generate_content.hpp"
#include "../../../interface/protocols/gemini/strings.hpp"
#include "base.hpp"
#include "../../curl.hpp"


namespace jai::llm {


BEGIN_DESERIALIZE(gemini::Blob)
    FIELD(src, mimeType),
    FIELD(src, data)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::Candidate)
    FIELD(src, content),
    FIELD(src, finishReason),
    FIELD(src, safetyRatings),
    FIELD(src, citationMetadata),
    FIELD(src, tokenCount),
    FIELD(src, groundingMetadata),
    FIELD(src, avgLogprobs),
    FIELD(src, logprobsResult),
    FIELD(src, urlContextMetadata),
    FIELD(src, index),
    FIELD(src, finishMessage)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::CitationMetadata)
    FIELD(src, citationSources)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::CitationMetadata::CitationSource)
    FIELD(src, startIndex),
    FIELD(src, endIndex),
    FIELD(src, uri),
    FIELD(src, license)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::ExecutableCode)
    FIELD(src, language),
    FIELD(src, code)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::FileData)
    FIELD(src, mimeType),
    FIELD(src, fileUri)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::FunctionCall)
    FIELD(src, name),
    FIELD(src, args),
    FIELD(src, id)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::GroundingChunk)
    FIELD_PLACEHOLDER(src, chunk_type)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::GroundingChunk::Maps)
    FIELD(src, uri),
    FIELD(src, title),
    FIELD(src, text),
    FIELD(src, placeId),
    FIELD(src, placeAnswerSources),
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::GroundingChunk::Maps::PlaceAnswerSources)
    FIELD(src, reviewSnippets)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::GroundingChunk::Maps::PlaceAnswerSources::ReviewSnippets)
    FIELD(src, reviewId),
    FIELD(src, googleMapsUri),
    FIELD(src, title)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::GroundingChunk::RetrievedContext)
    FIELD(src, uri),
    FIELD(src, title),
    FIELD(src, text),
    FIELD(src, fileSearchStore)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::GroundingChunk::Web)
    FIELD(src, uri),
    FIELD(src, title)
END_DESERIALIZE


BEGIN_DESERIALIZE_VARIANT(gemini::GroundingChunk::ChunkType)
    FIELD_VARIANT_KV(src, "maps",             gemini::GroundingChunk::Maps)
    FIELD_VARIANT_KV(src, "retrievedContext", gemini::GroundingChunk::RetrievedContext)
    FIELD_VARIANT_KV(src, "web",              gemini::GroundingChunk::Web)
END_DESERIALIZE_VARIANT(gemini::GroundingChunk::ChunkType)


BEGIN_DESERIALIZE(gemini::GroundingMetadata)
    FIELD(src, groundingChunks),
    FIELD(src, groundingSupports),
    FIELD(src, webSearchQueries),
    FIELD(src, searchEntryPoint),
    FIELD(src, retrievalMetadata),
    FIELD(src, googleMapsWidgetContextToken)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::GroundingMetadata::RetrievalMetadata)
    FIELD(src, googleSearchDynamicRetrievalScore)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::GroundingSupport)
    FIELD(src, groundingChunkIndices),
    FIELD(src, confidenceScores),
    FIELD(src, segment)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::GroundingSupport::Segment)
    FIELD(src, partIndex),
    FIELD(src, startIndex),
    FIELD(src, endIndex),
    FIELD(src, text)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::LogprobsResult)
    FIELD(src, topCandidates),
    FIELD(src, chosenCandidates),
    FIELD(src, logProbabilitySum)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::LogprobsResult::Candidate)
    FIELD(src, token),
    FIELD(src, tokenId),
    FIELD(src, logProbability)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::LogprobsResult::TopCandidate)
    FIELD(src, candidates)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::ModelStatus)
    FIELD(src, modelStage),
    FIELD(src, retirementTime),
    FIELD(src, message)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::PromptFeedback)
    FIELD(src, blockReason),
    FIELD(src, safetyRatings)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::Response)
    FIELD(src, candidates),
    FIELD(src, promptFeedback),
    FIELD(src, usageMetadata),
    FIELD(src, modelVersion),
    FIELD(src, responseId),
    FIELD(src, modelStatus)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::ResponseContent)
    FIELD(src, parts),
    FIELD(src, role)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::ResponseContent::ResponsePart)
    FIELD_PLACEHOLDER(src, data),
    FIELD(src, partMetadata),
    FIELD(src, thoughtSignature),
    FIELD(src, thought)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::SafetyRating)
    FIELD(src, category),
    FIELD(src, probability),
    FIELD(src, blocked)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::SearchEntryPoint)
    FIELD(src, renderedContent),
    FIELD(src, sdkBlob)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::Text)
    FIELD(src, text)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::UrlContextMetadata)
    FIELD(src, urlMetadata)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::UrlContextMetadata::UrlMetadata)
    FIELD(src, retrievedUrl),
    FIELD(src, urlRetrievalStatus)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::UsageMetadata)
    FIELD(src, promptTokenCount),
    FIELD(src, cachedContentTokenCount),
    FIELD(src, candidatesTokenCount),
    FIELD(src, toolUsePromptTokenCount),
    FIELD(src, thoughtsTokenCount),
    FIELD(src, totalTokenCount),
    FIELD(src, promptTokensDetails),
    FIELD(src, cacheTokensDetails),
    FIELD(src, candidatesTokensDetails),
    FIELD(src, toolUsePromptTokensDetails)
END_DESERIALIZE


BEGIN_DESERIALIZE(gemini::UsageMetadata::ModalityTokenCount)
    FIELD(src, modality),
    FIELD(src, tokenCount)
END_DESERIALIZE


BEGIN_DESERIALIZE_VARIANT(gemini::ResponseContent::ResponsePart::ResponsePartData)
    FIELD_VARIANT(src, "text",          gemini::Text)
    FIELD_VARIANT(src, "inlineData",    gemini::Blob)
    FIELD_VARIANT(src, "executableCode",gemini::ExecutableCode)
    FIELD_VARIANT(src, "fileData",      gemini::FileData)
    FIELD_VARIANT(src, "functionCall",  gemini::FunctionCall)
END_DESERIALIZE_VARIANT(gemini::ResponseContent::ResponsePart::ResponsePartData)


/***
 * Top-level Deserialize
 */
namespace gemini {
    Response Deserialize(const curl::Response& response) {
        if (response.body.size() < response.body_len + simdjson::SIMDJSON_PADDING) {
            throw AnnotatedException{"Simdjson padding check failed"};
        }

        static thread_local simdjson::dom::parser parser{};

        try {
            simdjson::dom::element doc = parser.parse(reinterpret_cast<const char*>(response.body.data()),
                                                    response.body_len);
            return DeserializeTo<Response>(doc);
        } catch (const simdjson::simdjson_error& e) {
            throw AnnotatedException{"Deserialize Failed", e.what()};
        }
    }
}

}
