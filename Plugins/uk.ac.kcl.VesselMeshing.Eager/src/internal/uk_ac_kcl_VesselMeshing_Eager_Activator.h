#pragma once

#include <ctkPluginActivator.h>

#include <IO/MeshDataIO.h>
#include <IO/MeshingParametersDataIO.h>

#include <memory>

class uk_ac_kcl_VesselMeshing_Eager_Activator : public QObject, public ctkPluginActivator
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "uk_ac_kcl_VesselMeshing_Eager")
    Q_INTERFACES(ctkPluginActivator)

public:

    void start(ctkPluginContext* context) override;
    void stop(ctkPluginContext* context) override;

    static ctkPluginContext* GetPluginContext() { return PluginContext; }

private:

    static ctkPluginContext* PluginContext;

    std::unique_ptr<crimson::MeshDataIO> _meshDataIO;
    std::unique_ptr<crimson::MeshingParametersDataIO> _meshingParametersDataIO;
}; // uk_ac_kcl_VesselMeshing_Eager_Activator
