#ifdef USE_XNNPACK

#include <ATen/core/op_registration/op_registration.h>
#include <ATen/native/xnnpack/Convolution.h>
#include <ATen/native/xnnpack/Linear.h>
#include <ATen/native/xnnpack/OpContext.h>
#include <ATen/Tensor.h>
#include <torch/custom_class.h>

namespace at {
namespace native {
namespace xnnpack {

namespace {
torch::jit::class_<XNNPackLinearOpContext> register_xnnpack_linear_op_context_class() {
  static int custom_class_handler = torch::jit::register_custom_class_handler();
  static auto register_linear_op_context_class =
      torch::jit::class_<XNNPackLinearOpContext>("XNNPackLinearOpContext")
          .def_pickle(
              [](const c10::intrusive_ptr<XNNPackLinearOpContext>& op_context)
                  -> SerializationTypeLinearPrePack { // __getstate__
                Tensor weight;
                c10::optional<Tensor> bias;
                std::tie(weight, bias) = op_context->unpack();
                return std::make_tuple(
                    std::move(weight),
                    std::move(bias));
              },
              [](SerializationTypeLinearPrePack state)
                  -> c10::intrusive_ptr<
                      XNNPackLinearOpContext> { // __setstate__
                return XNNPackLinearOpContext::create_context(
                    std::move(std::get<0>(state)),
                    std::move(std::get<1>(state)),
                    {},
                    {}
                    );
              }
              );
  return register_linear_op_context_class;
}

torch::jit::class_<XNNPackConv2dOpContext> register_xnnpack_conv2d_op_context_class() {
  static int custom_class_handler = torch::jit::register_custom_class_handler();
  static auto register_conv2d_op_context_class =
      torch::jit::class_<XNNPackConv2dOpContext>("XNNPackConv2dOpContext")
          .def_pickle(
              [](const c10::intrusive_ptr<XNNPackConv2dOpContext>& op_context)
                  -> SerializationTypeConv2dPrePack { // __getstate__
                Tensor weight;
                std::vector<int64_t> padding, stride, dilation;
                int64_t groups;
                c10::optional<Tensor> bias;
                std::tie(weight, bias, padding, stride, dilation, groups) =
                  op_context->unpack();
                return std::make_tuple(
                    std::move(weight),
                    std::move(bias),
                    std::move(padding),
                    std::move(stride),
                    std::move(dilation),
                    std::move(groups)
                    );
              },
              [](SerializationTypeConv2dPrePack state)
                  -> c10::intrusive_ptr<
                      XNNPackConv2dOpContext> { // __setstate__
                return XNNPackConv2dOpContext::create_context(
                    std::move(std::get<0>(state)),
                    std::move(std::get<1>(state)),
                    std::move(std::get<2>(state)),
                    std::move(std::get<3>(state)),
                    std::move(std::get<4>(state)),
                    std::move(std::get<5>(state)),
                    {},
                    {}
                    );
              }
              );
  return register_conv2d_op_context_class;
}

static auto xnnpack_linear_op_context_class = register_xnnpack_linear_op_context_class();
static auto xnnpack_conv2d_op_context_class = register_xnnpack_conv2d_op_context_class();

// Op registeration
static auto registry =
  // Registering under xnnpack namespace for now. As we add more backend requiring similar functionality
  // We can refactor the code and use a better namespace.
    torch::RegisterOperators()
        .op("xnnpack::linear_prepack(Tensor W, Tensor? B=None) -> __torch__.torch.classes.XNNPackLinearOpContext",
            torch::RegisterOperators::options()
            .aliasAnalysis(at::AliasAnalysisKind::PURE_FUNCTION)
            .kernel<internal::linear::LinearPrePack>(
                DispatchKey::CPUTensorId))
        .op("xnnpack::linear_packed(Tensor X, __torch__.torch.classes.XNNPackLinearOpContext W_prepack) -> Tensor Y",
            torch::RegisterOperators::options()
            .aliasAnalysis(at::AliasAnalysisKind::PURE_FUNCTION)
            .kernel<internal::linear::LinearPacked>(
                DispatchKey::CPUTensorId))
        .op("xnnpack::conv2d_prepack(Tensor W, Tensor? B, int[2] stride, "
            "int[2] padding, int[2] dilation, int groups) "
            "-> __torch__.torch.classes.XNNPackConv2dOpContext",
            torch::RegisterOperators::options()
            .aliasAnalysis(at::AliasAnalysisKind::PURE_FUNCTION)
            .kernel<internal::convolution2d::Conv2dPrePack>(
                DispatchKey::CPUTensorId))
        .op("xnnpack::conv2d_packed(Tensor X, "
            "__torch__.torch.classes.XNNPackConv2dOpContext W_prepack) -> Tensor Y",
            torch::RegisterOperators::options()
            .aliasAnalysis(at::AliasAnalysisKind::PURE_FUNCTION)
            .kernel<internal::convolution2d::Conv2dPacked>(
                DispatchKey::CPUTensorId));
} // namespace

} // xnnpack
} // native
} // at

namespace {
}

#endif /* USE_XNNPACK */
