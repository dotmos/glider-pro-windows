use super::{ColorTable, Point, Rect};
use crate::bitmap::{Bitmap, BitmapFour, BitmapOne, RgbQuad};
use crate::icocur::CursorFile;
use crate::utils::ReadExt;
use std::io::{self, ErrorKind, Read, Seek, SeekFrom, Write};

struct CCrsr {
    // CCrsr
    crsrType: u16,
    crsrMap: u32,
    crsrData: u32,
    crsrXData: u32,
    crsrXValid: u16,
    crsrXHandle: u32,
    crsr1Data: [u8; 32],
    crsrMask: [u8; 32],
    crsrHotSpot: Point,
    crsrXTable: u32,
    crsrID: u32,
    // PixMap
    baseAddr: u32,
    rowBytes: u16,
    bounds: Rect,
    pmVersion: u16,
    packType: u16,
    packSize: u32,
    hRes: i32,
    vRes: i32,
    pixelType: u16,
    pixelSize: u16,
    cmpCount: u16,
    cmpSize: u16,
    planeBytes: u32,
    pmTable: u32,
    pmReserved: u32,
    // pixel data
    pixelData: Vec<u8>,
    // color table data
    pixelCTab: ColorTable,
}

impl CCrsr {
    fn read_from(mut reader: impl Read + Seek) -> io::Result<Self> {
        let crsrType = reader.read_be_u16()?;
        let crsrMap = reader.read_be_u32()?;
        let crsrData = reader.read_be_u32()?;
        let crsrXData = reader.read_be_u32()?;
        let crsrXValid = reader.read_be_u16()?;
        let crsrXHandle = reader.read_be_u32()?;
        let mut crsr1Data = [0; 32];
        reader.read_exact(&mut crsr1Data)?;
        let mut crsrMask = [0; 32];
        reader.read_exact(&mut crsrMask)?;
        let crsrHotSpot = Point::read_from(&mut reader)?;
        let crsrXTable = reader.read_be_u32()?;
        let crsrID = reader.read_be_u32()?;

        reader.seek(SeekFrom::Start(crsrMap as _))?;
        let baseAddr = reader.read_be_u32()?;
        let rowBytes = reader.read_be_u16()?;
        let bounds = Rect::read_from(&mut reader)?;
        let pmVersion = reader.read_be_u16()?;
        let packType = reader.read_be_u16()?;
        let packSize = reader.read_be_u32()?;
        let hRes = reader.read_be_i32()?;
        let vRes = reader.read_be_i32()?;
        let pixelType = reader.read_be_u16()?;
        let pixelSize = reader.read_be_u16()?;
        let cmpCount = reader.read_be_u16()?;
        let cmpSize = reader.read_be_u16()?;
        let planeBytes = reader.read_be_u32()?;
        let pmTable = reader.read_be_u32()?;
        let pmReserved = reader.read_be_u32()?;

        reader.seek(SeekFrom::Start(crsrData as _))?;
        let mut pixelData = vec![0x00; pmTable.saturating_sub(crsrData) as _];
        reader.read_exact(&mut pixelData)?;

        reader.seek(SeekFrom::Start(pmTable as _))?;
        let pixelCTab = ColorTable::read_from(&mut reader)?;

        Ok(Self {
            crsrType,
            crsrMap,
            crsrData,
            crsrXData,
            crsrXValid,
            crsrXHandle,
            crsr1Data,
            crsrMask,
            crsrHotSpot,
            crsrXTable,
            crsrID,
            baseAddr,
            rowBytes,
            bounds,
            pmVersion,
            packType,
            packSize,
            hRes,
            vRes,
            pixelType,
            pixelSize,
            cmpCount,
            cmpSize,
            planeBytes,
            pmTable,
            pmReserved,
            pixelData,
            pixelCTab,
        })
    }
}

// HACK: In theory, the color table could contain any arrangement
// of colors. In practise, however, Macintosh color tables have
// a white entry at the first index and a black entry at the last
// index. To get black to be index 0, each pixel index is subtracted
// from the maximum value and the color table is reversed. This is
// done to meet Windows .ico conventions, where black is the first
// entry in the color palette.

pub fn convert(data: &[u8], writer: impl Write) -> io::Result<()> {
    let cursor = CCrsr::read_from(io::Cursor::new(data))?;
    let mut cur_file = CursorFile::new();

    let mut cursor_mask = BitmapOne::new(16, 16);
    cursor_mask.set_palette([RgbQuad::BLACK, RgbQuad::WHITE].iter().copied());
    super::read_mask_bitmap_data(&mut cursor_mask, &cursor.crsrMask, 0);

    match cursor.pixelSize {
        2 => {
            let mut palette = vec![RgbQuad::BLACK; 16];
            palette
                .iter_mut()
                .rev()
                .zip(&cursor.pixelCTab.ctTable)
                .for_each(|(dst, src)| *dst = src.rgb.into());
            let mut bitmap = BitmapFour::new(16, 16);
            bitmap.set_palette(palette.iter().copied());
            super::read_2bit_bitmap_data(&mut bitmap, &cursor.pixelData, 0);
            cur_file.add_entry(bitmap, cursor_mask.clone(), cursor.crsrHotSpot);
        }
        4 => {
            let mut palette = vec![RgbQuad::BLACK; 16];
            palette
                .iter_mut()
                .rev()
                .zip(&cursor.pixelCTab.ctTable)
                .for_each(|(dst, src)| *dst = src.rgb.into());
            let mut bitmap = BitmapFour::new(16, 16);
            bitmap.set_palette(palette.iter().copied());
            super::read_4bit_bitmap_data(&mut bitmap, &cursor.pixelData, 0);
            cur_file.add_entry(bitmap, cursor_mask.clone(), cursor.crsrHotSpot);
        }
        _ => {
            return Err(io::Error::new(
                ErrorKind::InvalidData,
                format!("'crsr': unsupported color depth ({})", cursor.pixelSize),
            ))
        }
    }

    let mut mono_bits = BitmapOne::new(16, 16);
    mono_bits.set_palette([RgbQuad::BLACK, RgbQuad::WHITE].iter().copied());
    super::read_1bit_bitmap_data(&mut mono_bits, &cursor.crsr1Data, 0);
    cur_file.add_entry(mono_bits, cursor_mask, cursor.crsrHotSpot);

    cur_file.write_to(writer)
}
