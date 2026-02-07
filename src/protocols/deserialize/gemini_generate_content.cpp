#include "../../../interface/protocols/gemini/generate_content.hpp"
#include "../../../interface//core/error.hpp"
#include "base.hpp"
#include "../../curl.hpp"


namespace jai::llm {


BEGIN_PARSE(gemini::Blob)
    FIELD(src, mimeType),
    FIELD(src, data)
END_PARSE


BEGIN_PARSE(gemini::Candidate)
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
END_PARSE


BEGIN_PARSE(gemini::CitationMetadata)
    FIELD(src, citationSources)
END_PARSE


BEGIN_PARSE(gemini::CitationMetadata::CitationSource)
    FIELD(src, startIndex),
    FIELD(src, endIndex),
    FIELD(src, uri),
    FIELD(src, license)
END_PARSE


BEGIN_PARSE(gemini::ExecutableCode)
    FIELD(src, language),
    FIELD(src, code)
END_PARSE


BEGIN_PARSE(gemini::FileData)
    FIELD(src, mimeType),
    FIELD(src, fileUri)
END_PARSE


BEGIN_PARSE(gemini::FunctionCall)
    FIELD(src, name),
    FIELD(src, args),
    FIELD(src, id)
END_PARSE


BEGIN_PARSE(gemini::GroundingChunk)
    FIELD(src, chunk_type)
END_PARSE


BEGIN_PARSE(gemini::GroundingChunk::Maps)
    FIELD(src, uri),
    FIELD(src, title),
    FIELD(src, text),
    FIELD(src, placeId),
    FIELD(src, placeAnswerSources),
END_PARSE


BEGIN_PARSE(gemini::GroundingChunk::Maps::PlaceAnswerSources)
    FIELD(src, reviewSnippets)
END_PARSE


BEGIN_PARSE(gemini::GroundingChunk::Maps::PlaceAnswerSources::ReviewSnippets)
    FIELD(src, reviewId),
    FIELD(src, googleMapsUri),
    FIELD(src, title)
END_PARSE


BEGIN_PARSE(gemini::GroundingChunk::RetrievedContext)
    FIELD(src, uri),
    FIELD(src, title),
    FIELD(src, text),
    FIELD(src, fileSearchStore)
END_PARSE


BEGIN_PARSE(gemini::GroundingChunk::Web)
    FIELD(src, uri),
    FIELD(src, title)
END_PARSE


BEGIN_PARSE(gemini::GroundingMetadata)
    FIELD(src, groundingChunks),
    FIELD(src, groundingSupports),
    FIELD(src, webSearchQueries),
    FIELD(src, searchEntryPoint),
    FIELD(src, retrievalMetadata),
    FIELD(src, googleMapsWidgetContextToken)
END_PARSE


BEGIN_PARSE(gemini::GroundingMetadata::RetrievalMetadata)
    FIELD(src, googleSearchDynamicRetrievalScore)
END_PARSE


BEGIN_PARSE(gemini::GroundingSupport)
    FIELD(src, groundingChunkIndices),
    FIELD(src, confidenceScores),
    FIELD(src, segment)
END_PARSE


BEGIN_PARSE(gemini::GroundingSupport::Segment)
    FIELD(src, partIndex),
    FIELD(src, startIndex),
    FIELD(src, endIndex),
    FIELD(src, text)
END_PARSE


BEGIN_PARSE(gemini::LogprobsResult)
    FIELD(src, topCandidates),
    FIELD(src, chosenCandidates),
    FIELD(src, logProbabilitySum)
END_PARSE


BEGIN_PARSE(gemini::LogprobsResult::Candidate)
    FIELD(src, token),
    FIELD(src, tokenId),
    FIELD(src, logProbability)
END_PARSE


BEGIN_PARSE(gemini::LogprobsResult::TopCandidate)
    FIELD(src, candidates)
END_PARSE


BEGIN_PARSE(gemini::ModelStatus)
    FIELD(src, modelStage),
    FIELD(src, retirementTime),
    FIELD(src, message)
END_PARSE


BEGIN_PARSE(gemini::PromptFeedback)
    FIELD(src, blockReason),
    FIELD(src, safetyRatings)
END_PARSE


BEGIN_PARSE(gemini::Response)
    FIELD(src, candidates),
    FIELD(src, promptFeedback),
    FIELD(src, usageMetadata),
    FIELD(src, modelVersion),
    FIELD(src, responseId),
    FIELD(src, modelStatus)
END_PARSE


BEGIN_PARSE(gemini::ResponseContent)
    FIELD(src, parts),
    FIELD(src, role)
END_PARSE


BEGIN_PARSE(gemini::ResponseContent::ResponsePart)
    FIELD(src, data),
    FIELD(src, partMetadata),
    FIELD(src, thoughtSignature),
    FIELD(src, thought)
END_PARSE


BEGIN_PARSE(gemini::SafetyRating)
    FIELD(src, category),
    FIELD(src, probability),
    FIELD(src, blocked)
END_PARSE


BEGIN_PARSE(gemini::SearchEntryPoint)
    FIELD(src, renderedContent),
    FIELD(src, sdkBlob)
END_PARSE


BEGIN_PARSE(gemini::Text)
    FIELD(src, text)
END_PARSE


BEGIN_PARSE(gemini::UrlContextMetadata)
    FIELD(src, urlMetadata)
END_PARSE


BEGIN_PARSE(gemini::UrlContextMetadata::UrlMetadata)
    FIELD(src, retrievedUrl),
    FIELD(src, urlRetrievalStatus)
END_PARSE


BEGIN_PARSE(gemini::UsageMetadata)
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
END_PARSE


BEGIN_PARSE(gemini::UsageMetadata::ModalityTokenCount)
    FIELD(src, modality),
    FIELD(src, tokenCount)
END_PARSE


template <>
gemini::GroundingChunk::ChunkType Parse<gemini::GroundingChunk::ChunkType>(const simdjson::dom::element& src) {
    using T = gemini::GroundingChunk::ChunkType;

    auto obj = src.get_object();
    if (auto r = ExtractForVariant<"maps">(obj); r.has_value()) {
        return T{Parse<gemini::GroundingChunk::Maps>(*r)};
    }
    if (auto r = ExtractForVariant<"retrievedContext">(obj); r.has_value()) {
        return T{Parse<gemini::GroundingChunk::RetrievedContext>(*r)};
    }
    if (auto r = ExtractForVariant<"web">(obj); r.has_value()) {
        return T{jai::llm::Parse<gemini::GroundingChunk::Web>(*r)};
    }
    throw AnnotatedException{"gemini::GroundingChunk::ChunkType variant unsatisfied"};
}


template <>
gemini::ResponseContent::ResponsePart::ResponsePartData
    Parse<gemini::ResponseContent::ResponsePart::ResponsePartData>(const simdjson::dom::element& src)
{
    using T = gemini::ResponseContent::ResponsePart::ResponsePartData;
    auto obj = src.get_object();
    if (auto r = ExtractForVariant<"text"          >(obj); r.has_value()) { return T{Parse<gemini::Text>(*r)}; }
    if (auto r = ExtractForVariant<"inlineData"    >(obj); r.has_value()) { return T{Parse<gemini::Blob>(*r)}; }
    if (auto r = ExtractForVariant<"executableCode">(obj); r.has_value()) { return T{Parse<gemini::ExecutableCode>(*r)}; }
    if (auto r = ExtractForVariant<"fileData"      >(obj); r.has_value()) { return T{Parse<gemini::FileData>(*r)}; }
    if (auto r = ExtractForVariant<"functionCall"  >(obj); r.has_value()) { return T{Parse<gemini::FunctionCall>(*r)}; }
    throw AnnotatedException{"gemini::ResponseContent::ResponsePart::ResponsePartData variant unsatisfied"};
}


} // namespace jai::llm


#undef FIELD
#undef BEGIN_PARSE
#undef END_PARSE


/***
 * Top-level Deserialize
 */
namespace jai::llm::gemini {


Response Deserialize(const curl::Response& response) {
    if (response.body.size() < response.body_len + simdjson::SIMDJSON_PADDING) {
        throw AnnotatedException{"Simdjson padding check failed"};
    }

    static thread_local simdjson::dom::parser parser{};

    try {
        simdjson::dom::element doc = parser.parse(reinterpret_cast<const char*>(response.body.data()),
                                                  response.body_len);
        return Parse<Response>(doc);
    } catch (const simdjson::simdjson_error& e) {
        throw AnnotatedException{"gemini::Deserialize Failed", e.what()};
    }
}


}
