using Flax.Build;

public class AmplitudeAudioEditorTarget : GameProjectEditorTarget
{
    /// <inheritdoc />
    public override void Init()
    {
        base.Init();

        Modules.Add("AmplitudeAudio");
        Modules.Add("AmplitudeAudioEditor");
    }
}
