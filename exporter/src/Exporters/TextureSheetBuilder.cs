using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using CTFAK.CCN;
using CTFAK.Core.CCN.Chunks.Banks.ImageBank;
using CTFAK.FileReaders;
using CTFAK.CCN.Chunks.Objects;
using RectpackSharp;

public class AtlasMetadata
{
	public int AtlasIndex { get; set; }
	public int X { get; set; }
	public int Y { get; set; }
}

public class TextureSheetBuilder
{
	public static Dictionary<int, AtlasMetadata> ImageAtlasMetadata
	{
		get { return _imageAtlasMetadata ??= new Dictionary<int, AtlasMetadata>(); }
	}
	private static Dictionary<int, AtlasMetadata>? _imageAtlasMetadata;

	public static List<Bitmap> TextureSheets
	{
		get
		{
			if (_textureSheets == null)
				throw new InvalidOperationException("TextureSheetBuilder.Initialize must be called before TextureSheets is used.");
			return _textureSheets;
		}
	}
	private static List<Bitmap>? _textureSheets;

	private const int MAX_TEXTURE_SHEET_SIZE = 512;
	private const int RECTANGLE_PADDING = 1;

	private class ImagePackingGroup
	{
		public string Name = "";
		public List<FusionImage> Images = new();
	}

	public static void Initialize(GameData gameData)
	{
		_imageAtlasMetadata = new Dictionary<int, AtlasMetadata>();
		_textureSheets = BuildTextureSheet(gameData);
	}

	private static List<Bitmap> BuildTextureSheet(GameData gameData)
	{
		var allValidImages = gameData.Images.Items.Values
			.Where(img => img.bitmap != null)
			.ToDictionary(img => img.Handle, img => img);

		var groups = BuildPackingGroups(gameData, allValidImages);

		return PackGroups(groups, allValidImages);
	}

	private static List<ImagePackingGroup> BuildPackingGroups(
		GameData gameData,
		Dictionary<int, FusionImage> allValidImages)
	{
		var groups = new List<ImagePackingGroup>();
		var alreadyGrouped = new HashSet<int>();

		AddAnimationDirectionGroups(gameData, allValidImages, groups, alreadyGrouped);

		AddLooseImageGroups(allValidImages, groups, alreadyGrouped);

		return groups;
	}

	private static void AddAnimationDirectionGroups(
		GameData gameData,
		Dictionary<int, FusionImage> allValidImages,
		List<ImagePackingGroup> groups,
		HashSet<int> alreadyGrouped)
	{
		foreach (var objectInfo in gameData.frameitems.Values)
		{
			if (objectInfo.ObjectType != 2)
				continue;

			if (objectInfo.properties is not ObjectCommon common)
				continue;

			if (common.Animations == null)
				continue;

			foreach (var sequencePair in common.Animations.AnimationDict)
			{
				int sequenceId = sequencePair.Key;
				var sequence = sequencePair.Value;

				foreach (var directionPair in sequence.DirectionDict)
				{
					int directionId = directionPair.Key;
					var direction = directionPair.Value;

					var images = direction.Frames
						.Distinct()
						.Where(id => !alreadyGrouped.Contains((int)id))
						.Where(id => allValidImages.ContainsKey((int)id))
						.Select(id => allValidImages[(int)id])
						.ToList();

					if (images.Count == 0)
						continue;

					foreach (var img in images)
						alreadyGrouped.Add(img.Handle);

					groups.Add(new ImagePackingGroup
					{
						Name = $"Object {objectInfo.handle} Sequence {sequenceId} Direction {directionId}",
						Images = images
					});
				}
			}
		}
	}

	private static void AddLooseImageGroups(
		Dictionary<int, FusionImage> allValidImages,
		List<ImagePackingGroup> groups,
		HashSet<int> alreadyGrouped)
	{
		foreach (var image in allValidImages.Values)
		{
			if (alreadyGrouped.Contains(image.Handle))
				continue;

			alreadyGrouped.Add(image.Handle);

			groups.Add(new ImagePackingGroup
			{
				Name = $"Loose Image {image.Handle}",
				Images = new List<FusionImage> { image }
			});
		}
	}

	private static List<Bitmap> PackGroups(
		List<ImagePackingGroup> groups,
		Dictionary<int, FusionImage> allValidImages)
	{
		var textureSheets = new List<Bitmap>();
		int atlasIndex = 0;

		var currentImages = new List<FusionImage>();

		foreach (var group in groups)
		{
			if (group.Images.Count == 0)
				continue;

			foreach (var oversized in group.Images.Where(IsOversized).ToList())
			{
				FlushCurrentAtlas(currentImages, textureSheets, ref atlasIndex);

				textureSheets.Add(BuildAtlas(new List<FusionImage> { oversized }, atlasIndex));
				atlasIndex++;

				group.Images.Remove(oversized);
			}

			if (group.Images.Count == 0)
				continue;

			var attempt = new List<FusionImage>(currentImages);
			attempt.AddRange(group.Images);

			if (CanPack(attempt))
			{
				currentImages.AddRange(group.Images);
			}
			else
			{
				FlushCurrentAtlas(currentImages, textureSheets, ref atlasIndex);

				if (CanPack(group.Images))
				{
					currentImages.AddRange(group.Images);
				}
				else
				{
					foreach (var image in group.Images)
					{
						var singleAttempt = new List<FusionImage>(currentImages) { image };

						if (!CanPack(singleAttempt))
							FlushCurrentAtlas(currentImages, textureSheets, ref atlasIndex);

						currentImages.Add(image);
					}
				}
			}
		}

		FlushCurrentAtlas(currentImages, textureSheets, ref atlasIndex);

		return textureSheets;
	}

	private static void FlushCurrentAtlas(
		List<FusionImage> currentImages,
		List<Bitmap> textureSheets,
		ref int atlasIndex)
	{
		if (currentImages.Count == 0)
			return;

		textureSheets.Add(BuildAtlas(currentImages, atlasIndex));
		atlasIndex++;
		currentImages.Clear();
	}

	private static bool IsOversized(FusionImage image)
	{
		int paddedW = image.bitmap.Width + RECTANGLE_PADDING * 2;
		int paddedH = image.bitmap.Height + RECTANGLE_PADDING * 2;

		return paddedW > MAX_TEXTURE_SHEET_SIZE ||
			   paddedH > MAX_TEXTURE_SHEET_SIZE;
	}

	private static bool CanPack(List<FusionImage> images)
	{
		if (images.Count == 0)
			return true;

		if (images.Any(IsOversized))
			return false;

		try
		{
			var rectangles = BuildRectangles(images);

			RectanglePacker.Pack(
				rectangles,
				out _,
				maxBoundsWidth: MAX_TEXTURE_SHEET_SIZE,
				maxBoundsHeight: MAX_TEXTURE_SHEET_SIZE
			);

			return true;
		}
		catch
		{
			return false;
		}
	}

	private static Bitmap BuildAtlas(List<FusionImage> images, int atlasIndex)
	{
		if (images.Count == 1 && IsOversized(images[0]))
		{
			var image = images[0];

			int atlasW = image.bitmap.Width + RECTANGLE_PADDING * 2;
			int atlasH = image.bitmap.Height + RECTANGLE_PADDING * 2;

			Bitmap oversizedAtlas = new Bitmap(
				atlasW,
				atlasH,
				PixelFormat.Format32bppArgb
			);

			using (Graphics g = Graphics.FromImage(oversizedAtlas))
			{
				g.Clear(Color.Transparent);

				g.DrawImage(
					image.bitmap,
					RECTANGLE_PADDING,
					RECTANGLE_PADDING,
					image.bitmap.Width,
					image.bitmap.Height
				);
			}

			ImageAtlasMetadata[image.Handle] = new AtlasMetadata
			{
				AtlasIndex = atlasIndex,
				X = RECTANGLE_PADDING,
				Y = RECTANGLE_PADDING
			};

			return oversizedAtlas;
		}

		var rectangles = BuildRectangles(images);

		RectanglePacker.Pack(
			rectangles,
			out PackingRectangle bounds,
			maxBoundsWidth: MAX_TEXTURE_SHEET_SIZE,
			maxBoundsHeight: MAX_TEXTURE_SHEET_SIZE
		);

		Bitmap atlas = new Bitmap(
			(int)bounds.Width,
			(int)bounds.Height,
			PixelFormat.Format32bppArgb
		);

		using (Graphics g = Graphics.FromImage(atlas))
		{
			g.Clear(Color.Transparent);

			for (int i = 0; i < rectangles.Length; i++)
			{
				var rectangle = rectangles[i];
				var image = images[(int)rectangle.Id];

				int drawX = (int)rectangle.X + RECTANGLE_PADDING;
				int drawY = (int)rectangle.Y + RECTANGLE_PADDING;

				g.DrawImage(
					image.bitmap,
					drawX,
					drawY,
					image.bitmap.Width,
					image.bitmap.Height
				);

				ImageAtlasMetadata[image.Handle] = new AtlasMetadata
				{
					AtlasIndex = atlasIndex,
					X = drawX,
					Y = drawY
				};
			}
		}

		return atlas;
	}

	private static PackingRectangle[] BuildRectangles(List<FusionImage> images)
	{
		var rectangles = new PackingRectangle[images.Count];

		for (int i = 0; i < images.Count; i++)
		{
			var image = images[i];

			rectangles[i] = new PackingRectangle(
				0,
				0,
				(uint)(image.bitmap.Width + RECTANGLE_PADDING * 2),
				(uint)(image.bitmap.Height + RECTANGLE_PADDING * 2),
				i
			);
		}

		return rectangles;
	}
}
