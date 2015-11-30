//
//  snowcrashtest.h
//  snowcrash
//
//  Created by Zdenek Nemec on 5/21/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#ifndef SNOWCRASH_SNOWCRASHTEST_H
#define SNOWCRASH_SNOWCRASHTEST_H

#include "catch.hpp"
#include "MarkdownParser.h"
#include "SectionParser.h"

namespace snowcrashtest {

    struct Models {

        snowcrash::ModelTable modelTable;
        snowcrash::ModelSourceMapTable modelSourceMapTable;
    };

    struct NamedTypes {

        mson::NamedTypeBaseTable baseTable;
        mson::NamedTypeDependencyTable dependencyTable;
    };

    template <typename T, typename PARSER>
    struct SectionParserHelper {

        static void parse(const mdp::ByteBuffer& source,
                          snowcrash::SectionType type,
                          const snowcrash::ParseResultRef<T>& out,
                          const snowcrash::BlueprintParserOptions& opts = 0,
                          const Models& models = Models(),
                          snowcrash::ParseResult<snowcrash::Blueprint>* bp = NULL,
                          const NamedTypes& namedTypes = NamedTypes()) {

            mdp::MarkdownParser markdownParser;
            mdp::MarkdownNode markdownAST;

            snowcrash::ParseResult<snowcrash::Blueprint> blueprint;
            snowcrash::ParseResult<snowcrash::Blueprint>* bppointer;

            markdownParser.parse(source, markdownAST);

            REQUIRE(!markdownAST.children().empty());

            if (bp == NULL) {
                bppointer = &blueprint;
            } else {
                bppointer = bp;
            }

            snowcrash::SectionParserData pd(opts, source, bppointer->node);
            mdp::BuildCharacterIndex(pd.sourceCharacterIndex, source);

            pd.sectionsContext.push_back(type);

            pd.modelTable.insert(models.modelTable.begin(), models.modelTable.end());
            pd.modelSourceMapTable.insert(models.modelSourceMapTable.begin(), models.modelSourceMapTable.end());

            pd.namedTypeBaseTable.insert(namedTypes.baseTable.begin(), namedTypes.baseTable.end());
            pd.namedTypeDependencyTable.insert(namedTypes.dependencyTable.begin(), namedTypes.dependencyTable.end());

            PARSER::parse(markdownAST.children().begin(),
                          markdownAST.children(),
                          pd,
                          out);
        }

        static void parseMSON(const mdp::ByteBuffer& source,
                              snowcrash::SectionType type,
                              const snowcrash::ParseResultRef<T>& out,
                              const snowcrash::BlueprintParserOptions& opts = 0,
                              const NamedTypes& namedTypes = NamedTypes()) {

            parse(source, type, out, opts, Models(), NULL, namedTypes);
        }

    };

    struct ModelHelper {

        /** Builds a Symbols entry for testing purposes */
        static void build(const mdp::ByteBuffer& name,
                          Models& models) {

            snowcrash::ResourceModel model;
            snowcrash::SourceMap<snowcrash::ResourceModel> modelSM;
            mdp::BytesRangeSet sourcemap;

            sourcemap.push_back(mdp::BytesRange(0, 1));

            model.description = "Foo";
            model.body = "Bar";

            modelSM.description.sourceMap = sourcemap;
            modelSM.body.sourceMap = sourcemap;

            models.modelTable[name] = model;
            models.modelSourceMapTable[name] = modelSM;
        }
    };

    struct NamedTypeHelper {

        /** Builds an named type entry for testing purposes */
        static void build(const mson::Literal& literal,
                          const mson::BaseType& baseType,
                          NamedTypes& namedTypes) {

            namedTypes.baseTable[literal] = baseType;
            namedTypes.dependencyTable[literal] = std::set<mson::Literal>();
        }
    };

    /**
     * \brief Helper to test signature parsing. Uses Blueprint as dummy type.
     */
    struct SignatureParserHelper {

        static scpl::Signature parse(const mdp::ByteBuffer& source,
                                     const snowcrash::ParseResultRef<snowcrash::Blueprint>& out,
                                     const scpl::SignatureTraits::Traits traits,
                                     const mdp::MarkdownNode* node = NULL) {

            mdp::MarkdownParser markdownParser;
            mdp::MarkdownNode markdownAST;

            snowcrash::ParseResult<snowcrash::Blueprint> blueprint;

            if (node == NULL) {
                markdownParser.parse(source, markdownAST);
            } else {
                markdownAST = *node;
            }

            REQUIRE(!markdownAST.children().empty());

            snowcrash::SectionParserData pd(0, source, blueprint.node);

            scpl::Signature signature;
            scpl::SignatureTraits signatureTraits(traits);

            signature = scpl::SignatureSectionProcessorBase<snowcrash::Blueprint>
                            ::parseSignature(markdownAST.children().begin(), pd, signatureTraits, out.report);

            return signature;
        }
    };

    /**
     * \brief Helper to test source maps
     */
    struct SourceMapHelper {

        /**
         * If 'nth' is not given, check that the given sourceMap is of size 1
         * and also check the first row of the given sourceMap with the given location & length.
         *
         * If 'nth' is given, check that particular row of the given sourceMap with the
         * given location & length.
         */
        static void check(mdp::BytesRangeSet& sourceMap, int loc, int len, size_t nth = 0) {

            if (nth == 0) {

                nth = 1;
                REQUIRE(sourceMap.size() == 1);
            }

            REQUIRE(sourceMap[nth - 1].location == loc);
            REQUIRE(sourceMap[nth - 1].length == len);
        }
    };
}

#endif
