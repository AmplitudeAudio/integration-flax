using Flax.Build;

public class AmplitudeAudioTarget : GameProjectTarget
{
    /// <inheritdoc />
    public override void Init()
    {
        base.Init();

        Modules.Add("AmplitudeAudio");
    }
}
