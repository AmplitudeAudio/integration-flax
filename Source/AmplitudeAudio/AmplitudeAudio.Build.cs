using Flax.Build;
using Flax.Build.NativeCpp;

public class AmplitudeAudio : GameModule
{
    public override void Setup(BuildOptions options)
    {
        base.Setup(options);
        BuildNativeCode = true;
        options.PublicDependencies.Add("AudioSystem");
    }
}
