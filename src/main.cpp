// For https://github.com/shader-slang/slang/issues/8882

#include <slang.h>
#include <slang-com-helper.h>
#include <slang-com-ptr.h>
#include <array>
#include <iostream>

const char* shaderSourceError = "struct Input\r\n{\r\n    uint VertexId : SV_VertexId;\r\n};\r\n\r\nstruct Output\r\n{\r\n    float4 Position : SV_Position;\r\n    float2 Uv : Uv;\r\n};\r\n\r\nvoid main(in Input input, out Output output)\r\n{\r\n    float4 positionUvs[3];\r\n    positionUvs[0] = float4(-1, -1, 0, 0);\r\n    positionUvs[1] = float4(3, -1, 2, 0);\r\n    positionUvs[2] = float4(-1, 3, 0, 2);\r\n\r\n    output.Position = float4(positionUvs[input.VertexId].xy, 0, 1);\r\n    output.Uv = float2(positionUvs[input.VertexId].zw);\r\n}";
const char* shaderSourceOk = "struct Input\r\n{\r\n    uint VertexId : SV_VertexId;\r\n};\r\n\r\nstruct Output\r\n{\r\n    float4 Position : SV_Position;\r\n    float2 Uv : Uv;\r\n};\r\n\r\n[shader(\"vertex\")] void main(in Input input, out Output output)\r\n{\r\n    float4 positionUvs[3];\r\n    positionUvs[0] = float4(-1, -1, 0, 0);\r\n    positionUvs[1] = float4(3, -1, 2, 0);\r\n    positionUvs[2] = float4(-1, 3, 0, 2);\r\n\r\n    output.Position = float4(positionUvs[input.VertexId].xy, 0, 1);\r\n    output.Uv = float2(positionUvs[input.VertexId].zw);\r\n}";

void logDiagnostics(slang::IBlob* diagnostics)
{
    if (diagnostics != nullptr)
    {
        std::cout << (const char*)diagnostics->getBufferPointer() << std::endl;
    }
}

int compile(const char* shaderSource, const char* shaderName)
{
    // Create global session
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    createGlobalSession(globalSession.writeRef());

    // Create target
    slang::TargetDesc targetDesc =
    {
        .format = SLANG_GLSL,
        .profile = globalSession->findProfile("spirv_1_5"),
    };

    // Create local session
    slang::SessionDesc localSessionDesc =
    {
        .targets = &targetDesc,
        .targetCount = 1,
    };

    Slang::ComPtr<slang::ISession> session;
    globalSession->createSession(localSessionDesc, session.writeRef());

    // Load module
    Slang::ComPtr<slang::IModule> slangModule;
    {
        Slang::ComPtr<slang::IBlob> diagnostics;
        slangModule = session->loadModuleFromSourceString("shader.slang","shader.slang", shaderSource,diagnostics.writeRef());
        logDiagnostics(diagnostics);
        if (!slangModule)
        {
            return -1;
        }
    }

    // Find entrypoint
    Slang::ComPtr<slang::IEntryPoint> entrypoint;
    {
        Slang::ComPtr<slang::IBlob> diagnostics;
        SlangResult result = slangModule->findAndCheckEntryPoint("main", SLANG_STAGE_VERTEX, entrypoint.writeRef(), diagnostics.writeRef());

        logDiagnostics(diagnostics);
        SLANG_RETURN_ON_FAIL(result);
    }

    // Compose program
    std::array<slang::IComponentType*, 2> components =
    {
        slangModule,
        entrypoint
    };

    Slang::ComPtr<slang::IComponentType> composedProgram;
    {
        Slang::ComPtr<slang::IBlob> diagnostics;
        SlangResult result = session->createCompositeComponentType(components.data(), components.size(), composedProgram.writeRef(), diagnostics.writeRef());

        logDiagnostics(diagnostics);
        SLANG_RETURN_ON_FAIL(result);
    }

    // Link
    Slang::ComPtr<slang::IComponentType> linkedProgram;
    {
        Slang::ComPtr<slang::IBlob> diagnostics;
        SlangResult result = composedProgram->link(linkedProgram.writeRef(), diagnostics.writeRef());

        logDiagnostics(diagnostics);
        SLANG_RETURN_ON_FAIL(result);
    }

    // Get code
    Slang::ComPtr<slang::IBlob> code;
    {
        Slang::ComPtr<slang::IBlob> diagnostics;
        SlangResult result = linkedProgram->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef());

        logDiagnostics(diagnostics);
        SLANG_RETURN_ON_FAIL(result);
    }

    std::cout << "Successfully compiled shader: " << shaderName << std::endl;

    if (targetDesc.format == SLANG_GLSL)
    {
        std::cout << static_cast<const char*>(code->getBufferPointer()) << std::endl;
    }

    return 0;
}

int main()
{
    compile(shaderSourceError, "error.slang");
    compile(shaderSourceOk, "ok.slang");

    return 0;
}
