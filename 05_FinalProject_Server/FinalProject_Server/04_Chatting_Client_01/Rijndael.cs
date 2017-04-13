using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace _04_Chatting_Client_01
{
	class Rijndael
	{
		//public static string Decrypt(string textToDecrypt, string key)
		//{
		//	RijndaelManaged rijndaelCipher = new RijndaelManaged();
		//	rijndaelCipher.Mode = CipherMode.CBC;
		//	rijndaelCipher.Padding = PaddingMode.PKCS7;

		//	rijndaelCipher.KeySize = 128;
		//	rijndaelCipher.BlockSize = 128;
		//	byte[] encryptedData = Convert.FromBase64String(textToDecrypt);
		//	byte[] pwdBytes = Encoding.UTF8.GetBytes(key);
		//	byte[] keyBytes = new byte[16];
		//	int len = pwdBytes.Length;
		//	if (len > keyBytes.Length)
		//	{
		//		len = keyBytes.Length;
		//	}
		//	Array.Copy(pwdBytes, keyBytes, len);
		//	rijndaelCipher.Key = keyBytes;
		//	rijndaelCipher.IV = keyBytes;
		//	byte[] plainText = rijndaelCipher.CreateDecryptor().TransformFinalBlock(encryptedData, 0, encryptedData.Length);
		//	return Encoding.UTF8.GetString(plainText);
		//}

		static byte[] iv = new byte[] { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

		public static byte[] Decrypt(byte[] buffer_cypher, int idx_start, int length_buffer, byte[] key)
		{
			RijndaelManaged rijndaelCipher = new RijndaelManaged();
			rijndaelCipher.Mode = CipherMode.CBC;
			rijndaelCipher.Padding = PaddingMode.PKCS7;

			rijndaelCipher.KeySize = 128;
			rijndaelCipher.BlockSize = 128;
			rijndaelCipher.Key = key;
			rijndaelCipher.IV = iv;
			ICryptoTransform transform = rijndaelCipher.CreateDecryptor();
			return transform.TransformFinalBlock(buffer_cypher, idx_start, length_buffer);
		}
		public static byte[] Encrypt(byte[] buffer_plain, int idx_start, int length_buffer, byte[] key)
		{
			RijndaelManaged rijndaelCipher = new RijndaelManaged();
			rijndaelCipher.Mode = CipherMode.CBC;
			rijndaelCipher.Padding = PaddingMode.PKCS7;

			rijndaelCipher.KeySize = 128;
			rijndaelCipher.BlockSize = 128;
			rijndaelCipher.Key = key;
			rijndaelCipher.IV = iv;
			ICryptoTransform transform = rijndaelCipher.CreateEncryptor();
			return transform.TransformFinalBlock(buffer_plain, idx_start, length_buffer);
		}
	}
}
