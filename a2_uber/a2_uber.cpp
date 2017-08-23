#include "bsdf_diffuse.hpp"
#include "bsdf_ggx_multiscatter.hpp"
#include "bsdf_microfacet.hpp"
#include "bsdf_microfacet_refraction.hpp"
#include "bsdf_stack.hpp"
#include "common/util.hpp"
#include <ai.h>
#include <iostream>
#include <spdlog/fmt/ostr.h>

AI_SHADER_NODE_EXPORT_METHODS(A2UberMtd);

class Uber {

public:
    [[ a2::label("Param 1"), 
       a2::help("blah blah blah") 
       a2::page("Diffuse/Advanced"]] 
    float param_1;
};

node_parameters {
    AiParameterFlt("roughness", 0.0f);
    AiParameterBool("compensate", true);
}

node_initialize {}

node_update {}

node_finish {}

shader_evaluate {
    using namespace a2;
    auto U = sg->dPdu;
    auto V = sg->dPdv;
    if (AiV3IsSmall(U)) {
        AiV3BuildLocalFrame(U, V, sg->Nf);
    }

    float roughness = sqr(AiShaderEvalParamFlt(0));
    bool compensate = AiShaderEvalParamBool(1);

    // auto bsdf_microfacet_reflection = create_microfacet_dielectric(
    // sg, AtRGB(1), sg->Nf, sg->dPdu, 1.0f, 1.5f, roughness, roughness);
    auto reflectivity = AtRGB(0.99, 0.791587, 0.3465);
    AtRGB edgetint = AtRGB(0.99, 0.9801, 0.792);

    auto bsdf_microfacet_reflection = create_microfacet_conductor(
        sg, AtRGB(1), sg->Nf, sg->dPdu, reflectivity, edgetint, roughness,
        roughness);

    // auto bsdf_microfacet_refraction = BsdfMicrofacetRefraction::create(
    // sg, AtRGB(1), sg->Nf, sg->dPdu, 1.0f, 1.5f, 0, 0);

    // auto bsdf_oren_nayar =
    // BsdfDiffuse::create(sg, AtRGB(0.18f), sg->Nf, sg->dPdu, 0.0f);

    // auto bsdf_stack = BsdfStack::create(sg);
    // bsdf_stack->add_bsdf(bsdf_microfacet_dielectric);
    // bsdf_stack->add_bsdf(bsdf_oren_nayar);
    // bsdf_stack->add_bsdf(bsdf_microfacet_refraction);
    // sg->out.CLOSURE() = bsdf_stack->get_arnold_bsdf();
    auto clist = AtClosureList();
    clist.add(bsdf_microfacet_reflection->get_arnold_bsdf());
    if (compensate) {
        auto bsdf_ms = create_ggx_ms_conductor(sg, sg->Nf, roughness,
                                               reflectivity, edgetint);
        // clist.add(AiOrenNayarBSDF(sg, AtRGB(ms_compensation), sg->Nf));
        clist.add(bsdf_ms->get_arnold_bsdf());
    }
    sg->out.CLOSURE() = clist;
}

node_loader {
    if (i > 0)
        return false;

    node->methods = A2UberMtd;
    node->output_type = AI_TYPE_CLOSURE;
    node->name = "a2_uber";
    node->node_type = AI_NODE_SHADER;
    strcpy(node->version, AI_VERSION);
    return true;
}
