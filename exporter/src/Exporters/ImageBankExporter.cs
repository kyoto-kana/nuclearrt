using System.Text;
using System.IO;

public class ImageBankExporter : BaseExporter
{
	public ImageBankExporter(Exporter exporter) : base(exporter) { }

	public override void Export()
	{
		TextureSheetBuilder.Initialize(GameData);

		var imageBankPath = Path.Combine(RuntimeBasePath.FullName, "source", "ImageBank.template.cpp");
		var imageBank = File.ReadAllText(imageBankPath);

		int imageCount = GameData.Images.Items.Values.Count;
		var imageBankData = new StringBuilder(imageCount * 80);

		foreach (var image in GameData.Images.Items.Values)
		{
			int mosaicIndex = 0;
			int mosaicX = 0;
			int mosaicY = 0;

			if (TextureSheetBuilder.ImageAtlasMetadata.TryGetValue(image.Handle, out var metadata))
			{
				mosaicIndex = metadata.AtlasIndex;
				mosaicX = metadata.X;
				mosaicY = metadata.Y;
			}

			imageBankData.Append("\t\t{ ")
						 .Append(image.Handle).Append(", ")
						 .Append(image.Width).Append(", ")
						 .Append(image.Height).Append(", ")
						 .Append(image.HotspotX).Append(", ")
						 .Append(image.HotspotY).Append(", ")
						 .Append(image.ActionX).Append(", ")
						 .Append(image.ActionY).Append(", ")
						 .Append(mosaicIndex).Append(", ")
						 .Append(mosaicX).Append(", ")
						 .Append(mosaicY).Append(", ")
						 .Append(ColorToRGB(image.Transparent)).Append(" },\n");
		}

		imageBank = imageBank.Replace("{{ IMAGES }}", imageBankData.ToString());

		SaveFile(Path.Combine(OutputPath.FullName, "source", "ImageBank.cpp"), imageBank);
		File.Delete(Path.Combine(OutputPath.FullName, "source", "ImageBank.template.cpp"));
	}
}
