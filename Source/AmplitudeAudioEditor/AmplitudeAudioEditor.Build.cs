using Flax.Build;
using Flax.Build.NativeCpp;

public class AmplitudeAudioEditor : GameEditorModule
{
    public override void Setup(BuildOptions options)
    {
        base.Setup(options);
        
        BuildNativeCode = false;
        
        options.PublicDependencies.Add("AmplitudeAudio");
        options.PublicDependencies.Add("Editor");
    }
}
