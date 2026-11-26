using CTFAK.CCN;
using CTFAK.Core.CCN.Chunks.Banks.ImageBank;
using CTFAK.FileReaders;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using System.Runtime.InteropServices;
using static CollisionMaskBuilder;

public class CollisionMaskBuilder
{
	public struct CollisionMask
	{
		public int Handle;
		public int Width;
		public int Height;
		public byte[] Data;
	}

	public struct CollisionPolygon
	{
		public List<System.Drawing.Point> Points;
	}

	public struct CollisionPolygonData
	{
		public int Handle;
		public byte[] Data;
	}
	private const int ALPHA_THRESHOLD = 128; //TODO: verify if this is accurate
	private static bool[] BuildAlphaMask(Bitmap bitmap)
	{
		int width = bitmap.Width;
		int height = bitmap.Height;
		var opaque = new bool[width * height];
		var data = bitmap.LockBits(new Rectangle(0, 0, width, height), ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
		try
		{
			var row = new byte[width * 4];

			for (int y = 0; y < height; y++)
			{
				Marshal.Copy(IntPtr.Add(data.Scan0, y * data.Stride), row, 0, row.Length);

				for (int x = 0; x < width; x++)
				{
					opaque[y * width + x] = row[x * 4 + 3] > ALPHA_THRESHOLD;
				}
			}
		}
		finally
		{
			bitmap.UnlockBits(data);
		}

		return opaque;
	}

	private static List<CollisionPolygon> GreedyCollision(bool[] alpha, int width, int height)
	{
		var polygons = new List<CollisionPolygon>();

		int n = 0;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width;)
			{
				if (alpha[n])
				{
					int w = 1;
					while (x + w < width && alpha[n + w])
					{
						w++;
					}

					int h = 1;
					bool done = false;
					while (y + h < height)
					{
						for (int k = 0; k < w; k++)
						{
							if (!alpha[n + k + h * width])
							{
								done = true;
								break;
							}
						}
						if (done) break;
						h++;
					}
					polygons.Add(new CollisionPolygon { Points = new List<System.Drawing.Point> { new(x, y), new(x + w, y + h) } });

					for (int l = 0; l < h; l++)
					{
						for (int k = 0; k < w; k++)
						{
							alpha[n + k + l * width] = false;
						}
					}

					x += w;
					n += w;
				}
				else
				{
					x++;
					n++;
				}
			}
		}
		return polygons;
	}

	public static CollisionPolygonData BuildCollisionPolygons(FusionImage image, byte[] imageData)
	{
		var bitmap = image.bitmap;

		var opaque = BuildAlphaMask(bitmap);

		var polygons = GreedyCollision(opaque, bitmap.Width, bitmap.Height);

		using var ms = new MemoryStream();
		using var writer = new BinaryWriter(ms);

		writer.Write(polygons.Count);

		foreach (var polygon in polygons)
		{
			writer.Write(polygon.Points.Count);

			foreach (var point in polygon.Points)
			{
				writer.Write(point.X);
				writer.Write(point.Y);
			}
		}

		return new CollisionPolygonData
		{
			Handle = image.Handle,
			Data = ms.ToArray()
		};
	}
}
