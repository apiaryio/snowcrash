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

        snowcrash::ModelTable models;
        snowcrash::ModelSourceMapTable modelsSM;
    };

    struct NamedTypes {

        mson::NamedTypeBaseTable bases;
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

            pd.sectionsContext.push_back(type);

            pd.modelTable.insert(models.models.begin(), models.models.end());
            pd.modelSourceMapTable.insert(models.modelsSM.begin(), models.modelsSM.end());

            pd.namedTypeBaseTable.insert(namedTypes.bases.begin(), namedTypes.bases.end());

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

    struct BuildHelper {

        /** Builds a Symbols entry for testing purposes */
        static void model(const mdp::ByteBuffer& name,
                          Models& models) {

            snowcrash::ResourceModel model;
            snowcrash::SourceMap<snowcrash::ResourceModel> modelSM;
            mdp::BytesRangeSet sourcemap;

            sourcemap.push_back(mdp::BytesRange(0, 1));

            model.description = "Foo";
            model.body = "Bar";
            models.models[name] = model;

            modelSM.description.sourceMap = sourcemap;
            modelSM.body.sourceMap = sourcemap;
            models.modelsSM[name] = modelSM;
        }

        /** Builds an named type entry for testing purposes */
        static void namedType(const mson::Literal& literal,
                              const mson::BaseType& baseType,
                              NamedTypes& namedTypes) {

            namedTypes.bases[literal] = baseType;
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
     * \brief Helper to test if part of MSON AST is empty
     */
    struct MSONHelper {

        /** Value */
        static void empty(const mson::Value& value) {

            REQUIRE(value.literal.empty());
            REQUIRE(value.variable == false);
        }

        /** Symbol */
        static void empty(const mson::Symbol& symbol) {

            REQUIRE(symbol.literal.empty());
            REQUIRE(symbol.variable == false);
        }

        /** Type Name */
        static void empty(const mson::TypeName& typeName) {

            REQUIRE(typeName.name == mson::UndefinedTypeName);
            MSONHelper::empty(typeName.symbol);
        }

        /** Type Specification */
        static void empty(const mson::TypeSpecification& typeSpecification) {

            REQUIRE(typeSpecification.nestedTypes.empty());
            MSONHelper::empty(typeSpecification.name);
        }

        /** Type Definition */
        static void empty(const mson::TypeDefinition& typeDefinition) {

            REQUIRE(typeDefinition.attributes == 0);
            MSONHelper::empty(typeDefinition.typeSpecification);
        }

        /** Value Definition */
        static void empty(const mson::ValueDefinition& valueDefinition) {

            REQUIRE(valueDefinition.values.empty());
            MSONHelper::empty(valueDefinition.typeDefinition);
        }

        /** Type Section */
        static void empty(const mson::TypeSection& typeSection) {

            REQUIRE(typeSection.type == mson::UndefinedTypeSectionType);
            REQUIRE(typeSection.content.description.empty());
            REQUIRE(typeSection.content.members().empty());
        }

        /** Value Member */
        static void empty(const mson::ValueMember& valueMember) {

            REQUIRE(valueMember.description.empty());
            REQUIRE(valueMember.sections.empty());
            MSONHelper::empty(valueMember.valueDefinition);
        }

        /** Property Member */
        static void empty(const mson::PropertyMember& propertyMember) {

            REQUIRE(propertyMember.name.literal.empty());
            MSONHelper::empty(propertyMember.name.variable);
            REQUIRE(propertyMember.description.empty());
            REQUIRE(propertyMember.sections.empty());
            MSONHelper::empty(propertyMember.valueDefinition);
        }

        /** Mixin */
        static void empty(const mson::Mixin& mixin) {

            MSONHelper::empty(mixin.typeDefinition);
        }

        /** One Of */
        static void empty(const mson::OneOf& oneOf) {

            REQUIRE(oneOf.members().empty());
        }
    };
}

#endif
