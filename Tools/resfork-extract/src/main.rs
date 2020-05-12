mod apple_double;
mod bitmap;
mod icocur;
mod mac_clut;
mod mac_roman;
mod macbinary;
mod res;
mod rsrcfork;
mod utils;

use crate::apple_double::AppleDouble;
use crate::bitmap::{Bitmap, BitmapEight, BitmapFour, BitmapOne};
use crate::icocur::IconFile;
use crate::macbinary::MacBinary;
use crate::res::*;
use crate::rsrcfork::{Resource, ResourceFork};
use std::collections::{HashMap, HashSet};
use std::env;
use std::error::Error;
use std::ffi::OsString;
use std::fs::File;
use std::io::prelude::*;
use std::io::{self, BufReader, BufWriter, Cursor, ErrorKind, SeekFrom};
use std::path::Path;
use std::process;
use std::sync::atomic::{AtomicUsize, Ordering};
use zip::ZipWriter;

pub type AnyResult<T> = Result<T, Box<dyn Error>>;

fn extract_resource_bytes(mut reader: impl Read + Seek) -> io::Result<Vec<u8>> {
    reader.seek(SeekFrom::Start(0))?;
    if let Ok(Some(data)) = AppleDouble::read_from(&mut reader) {
        return Ok(data.rsrc);
    }
    reader.seek(SeekFrom::Start(0))?;
    if let Ok(Some(data)) = MacBinary::read_from(&mut reader) {
        return Ok(data.rsrc);
    }
    reader.seek(SeekFrom::Start(0))?;
    let mut bytes = Vec::new();
    reader.read_to_end(&mut bytes).map(|_| bytes)
}

fn unprintable_to_period(b: u8) -> u8 {
    match b {
        0x00..=0x1F | 0x7F => b'.',
        _ => b,
    }
}

fn hex_byte_line(bytes: &[u8]) -> String {
    let mut line = String::from("$\"");
    for pair in bytes.chunks(2) {
        match pair.len() {
            1 => line.push_str(&format!("{:02X} ", pair[0])),
            2 => line.push_str(&format!("{:02X}{:02X} ", pair[0], pair[1])),
            _ => unreachable!(),
        }
    }
    let _space = line.pop();
    line.push('"');
    let bytes_string = bytes
        .iter()
        .copied()
        .map(unprintable_to_period)
        .map(mac_roman::decode)
        .collect::<String>();
    format!("\t{: <53} /* {} */", line, bytes_string)
}

fn escape_squote(unescaped: &str) -> String {
    let mut escaped = String::with_capacity(unescaped.len());
    for c in unescaped.chars() {
        if c == '\'' {
            escaped.push('\\');
        }
        escaped.push(c);
    }
    escaped
}

fn escape_dquote(unescaped: &str) -> String {
    let mut escaped = String::with_capacity(unescaped.len());
    for c in unescaped.chars() {
        if c == '\"' {
            escaped.push('\\');
        }
        escaped.push(c);
    }
    escaped
}

fn derez_resfork(resfork: &ResourceFork, mut writer: impl Write) -> AnyResult<()> {
    for res in resfork.iter() {
        let restype_str = escape_squote(&res.restype.to_string());
        write!(&mut writer, "data '{}' ({}", restype_str, res.id)?;
        if let Some(name) = &res.name {
            write!(&mut writer, ", \"{}\"", escape_dquote(name))?;
        }
        writeln!(&mut writer, ") {{")?;
        for row in res.data.chunks(16) {
            writeln!(&mut writer, "{}", hex_byte_line(row))?;
        }
        writeln!(&mut writer, "}};\n")?;
    }
    Ok(())
}

fn make_zip_entry_path(res: &Resource) -> String {
    let typ = res.restype;
    let id = res.id;
    fn valid_char(b: u8) -> bool {
        b.is_ascii_alphanumeric() || b"-_.#() ".contains(&b)
    }
    if typ.bytes.iter().copied().all(valid_char) {
        return format!("({})/{}.bin", typ, id);
    } else {
        return format!(
            "({:02X}-{:02X}-{:02X}-{:02X})/{}.bin",
            u32::from(typ.bytes[0]),
            u32::from(typ.bytes[1]),
            u32::from(typ.bytes[2]),
            u32::from(typ.bytes[3]),
            id
        );
    }
}

fn write_names_txt(resfork: &ResourceFork, mut writer: impl Write) -> io::Result<()> {
    for res in resfork.iter() {
        if let Some(name) = &res.name {
            writeln!(&mut writer, "('{}', {}): {:?}", res.restype, res.id, name)?;
        }
    }
    Ok(())
}

fn dump_resfork(resfork: &ResourceFork, writer: impl Seek + Write) -> AnyResult<()> {
    let mut zip_writer = ZipWriter::new(writer);
    zip_writer.set_comment("");

    if resfork.iter().any(|res| res.name.is_some()) {
        zip_writer.start_file("names.txt", Default::default())?;
        write_names_txt(resfork, &mut zip_writer)?;
    }

    for res in resfork.iter() {
        let entry_name = make_zip_entry_path(&res);
        zip_writer.start_file(entry_name, Default::default())?;
        zip_writer.write_all(res.data.as_slice())?;
    }
    Ok(())
}

fn get_entry_name(res: &Resource) -> String {
    match res.restype.as_bstr() {
        b"acur" => format!("AnimatedCursor/{}.txt", res.id),
        b"ALRT" => format!("Alert/{}.txt", res.id),
        b"BNDL" => format!("Bundle/{}.txt", res.id),
        b"cctb" => format!("ControlColorTable/{}.txt", res.id),
        b"CDEF" => format!("ControlDefinitionFunction/{}.bin", res.id),
        b"cicn" => format!("AppIcon/{}-color.ico", res.id),
        b"clut" => format!("ColorTable/{}.txt", res.id),
        b"CNTL" => format!("ControlTemplate/{}.txt", res.id),
        b"crsr" => format!("Cursor/{}-color.cur", res.id),
        b"CURS" => format!("Cursor/{}-mono.cur", res.id),
        b"dctb" => format!("DialogColorTable/{}.txt", res.id),
        b"demo" => format!("DemoData/{}.bin", res.id),
        b"DITL" => format!("ItemList/{}.txt", res.id),
        b"DLOG" => format!("Dialog/{}.txt", res.id),
        b"FREF" => format!("FileReference/{}.txt", res.id),
        b"ICON" => format!("AppIcon/{}-mono.ico", res.id),
        b"icl8" => format!("FinderIcon/{}-large-8bit.ico", res.id),
        b"icl4" => format!("FinderIcon/{}-large-4bit.ico", res.id),
        b"ICN#" => format!("FinderIcon/{}-large-mono.ico", res.id),
        b"ics8" => format!("FinderIcon/{}-small-8bit.ico", res.id),
        b"ics4" => format!("FinderIcon/{}-small-4bit.ico", res.id),
        b"ics#" => format!("FinderIcon/{}-small-mono.ico", res.id),
        b"mctb" => format!("MenuColorTable/{}.txt", res.id),
        b"MENU" => format!("Menu/{}.txt", res.id),
        b"PICT" => format!("Picture/{}.bmp", res.id),
        b"snd " => format!("Sound/{}.aif", res.id),
        b"STR#" => format!("StringList/{}.txt", res.id),
        b"vers" => format!("Version/{}.txt", res.id),
        b"wctb" => format!("WindowColorTable/{}.txt", res.id),
        b"WDEF" => format!("WindowDefinitionFunction/{}.bin", res.id),
        b"WIND" => format!("Window/{}.txt", res.id),
        _ => make_zip_entry_path(res),
    }
}

static EXIT_CODE: AtomicUsize = AtomicUsize::new(0);

fn report_invalid_resource(resource: &Resource) {
    EXIT_CODE.store(1, Ordering::SeqCst);
    eprintln!(
        "error: resource '{}' ({}) is invalid",
        resource.restype, resource.id
    );
}

fn convert_resource(resource: &Resource, mut writer: impl Write) -> io::Result<()> {
    let result = match resource.restype.as_bstr() {
        b"acur" => animated_cursor::convert(&resource.data, writer),
        b"ALRT" => alert::convert(&resource.data, writer),
        b"BNDL" => bundle::convert(&resource.data, writer),
        b"cctb" => control_color_table::convert(&resource.data, writer),
        b"CDEF" => writer.write_all(&resource.data),
        b"cicn" => color_icon::convert(&resource.data, writer),
        b"clut" => color_table::convert(&resource.data, writer),
        b"CNTL" => control::convert(&resource.data, writer),
        b"crsr" => color_cursor::convert(&resource.data, writer),
        b"CURS" => cursor::convert(&resource.data, writer),
        b"dctb" => dialog_color_table::convert(&resource.data, writer),
        b"demo" => writer.write_all(&resource.data),
        b"DITL" => item_list::convert(&resource.data, writer),
        b"DLOG" => dialog::convert(&resource.data, writer),
        b"FREF" => file_reference::convert(&resource.data, writer),
        b"ICON" => icon::convert(&resource.data, writer),
        b"icl8" => large_8bit_icon::convert(&resource.data).and_then(|image| {
            let mask = BitmapOne::new(32, 32);
            IconFile::new().add_entry(image, mask).write_to(writer)
        }),
        b"icl4" => large_4bit_icon::convert(&resource.data).and_then(|image| {
            let mask = BitmapOne::new(32, 32);
            IconFile::new().add_entry(image, mask).write_to(writer)
        }),
        b"ICN#" => icon_list::convert(&resource.data).and_then(|(image, mask)| {
            IconFile::new().add_entry(image, mask).write_to(writer)
        }),
        b"ics8" => small_8bit_icon::convert(&resource.data).and_then(|image| {
            let mask = BitmapOne::new(16, 16);
            IconFile::new().add_entry(image, mask).write_to(writer)
        }),
        b"ics4" => small_4bit_icon::convert(&resource.data).and_then(|image| {
            let mask = BitmapOne::new(16, 16);
            IconFile::new().add_entry(image, mask).write_to(writer)
        }),
        b"ics#" => small_icon_list::convert(&resource.data).and_then(|(image, mask)| {
            IconFile::new().add_entry(image, mask).write_to(writer)
        }),
        b"mctb" => menu_color_table::convert(&resource.data, writer),
        b"MENU" => menu::convert(&resource.data, writer),
        b"PICT" => picture::convert(&resource.data, writer),
        b"snd " => sound::convert(&resource.data, writer),
        b"STR#" => string_list::convert(&resource.data, writer),
        b"vers" => version::convert(&resource.data, writer),
        b"wctb" => window_color_table::convert(&resource.data, writer),
        b"WDEF" => writer.write_all(&resource.data),
        b"WIND" => window::convert(&resource.data, writer),
        // do not report the failure to convert unknown resources
        _ => return Err(ErrorKind::InvalidData.into()),
    };
    if result.is_err() {
        report_invalid_resource(resource);
    }
    result
}

fn get_finder_icon_ids(resfork: &ResourceFork) -> Vec<i16> {
    let mut ids = HashSet::new();
    for resource in resfork.iter() {
        match resource.restype.as_bstr() {
            b"icl8" | b"icl4" | b"ICN#" | b"ics8" | b"ics4" | b"ics#" => {
                ids.insert(resource.id);
            }
            _ => {}
        }
    }
    ids.into_iter().collect()
}

fn write_rc_script<W: Write>(resfork: &ResourceFork, mut writer: W) -> io::Result<()> {
    let icon_ids = get_finder_icon_ids(resfork);
    writeln!(&mut writer, "#pragma code_page(65001)")?;
    writeln!(&mut writer, "#include <windows.h>")?;
    writeln!(&mut writer, "LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US")?;
    for id in icon_ids.iter().copied() {
        let uid = id as u16;
        writeln!(&mut writer, r#"{0} ICON "build\\icon_{1}.ico""#, uid, id)?;
    }
    for id in resfork.iter_type(b"bnds").map(|r| r.id) {
        let uid = id as u16;
        writeln!(&mut writer, r#"{0} BOUNDS "build\\bnds_{1}.bin""#, uid, id)?;
    }
    for id in resfork.iter_type(b"PICT").map(|r| r.id) {
        let uid = id as u16;
        writeln!(&mut writer, r#"{0} BITMAP "build\\pict_{1}.bmp""#, uid, id)?;
    }
    for id in resfork.iter_type(b"snd ").map(|r| r.id) {
        let uid = id as u16;
        writeln!(&mut writer, r#"{0} WAVE "build\\snd_{1}.wav""#, uid, id)?;
    }
    Ok(())
}

fn write_do_rle_script<W: Write>(mut writer: W) -> io::Result<()> {
    writeln!(&mut writer, "magick convert %1 \"bmp3:%~2\"")?;
    writeln!(&mut writer, "if %~z1 leq %~z2 copy %1 %2 >nul")?;
    Ok(())
}

fn write_build_script<W: Write>(resfork: &ResourceFork, mut writer: W) -> io::Result<()> {
    let icon_ids = get_finder_icon_ids(resfork);
    writeln!(&mut writer, "@echo off")?;
    writeln!(
        &mut writer,
        "rem This build script uses ImageMagick and FFmpeg for conversion"
    )?;
    writeln!(
        &mut writer,
        r#"if not exist "%~dp0build" mkdir "%~dp0build""#
    )?;
    for id in &icon_ids {
        writeln!(
            &mut writer,
            r#"copy "%~dp0FinderIcon\{0}.ico" "%~dp0build\icon_{0}.ico" >nul"#,
            id
        )?;
    }
    for resource in resfork.iter_type(b"bnds") {
        writeln!(
            &mut writer,
            r#"copy "%~dp0(bnds)\{0}.bin" "%~dp0build\bnds_{0}.bin" >nul"#,
            resource.id
        )?;
    }
    writeln!(&mut writer, "echo Processing bitmap images...")?;
    for res in resfork.iter_type(b"PICT") {
        writeln!(
            &mut writer,
            r#"call "%~dp0do_rle.cmd" "%~dp0Picture\{0}.bmp" "%~dp0build\pict_{0}.bmp""#,
            res.id
        )?;
    }
    writeln!(&mut writer, "echo Processing audio files...")?;
    for res in resfork.iter_type(b"snd ") {
        writeln!(
            &mut writer,
            r#"ffmpeg -v quiet -y -i "%~dp0Sound\{0}.aif" -acodec pcm_u8 "%~dp0build\snd_{0}.wav""#,
            res.id
        )?;
    }
    writeln!(&mut writer, "echo Compiling resource script...")?;
    writeln!(
        &mut writer,
        r#"rc /nologo /fo "%~dp0resource.res" "%~dp0resource.rc""#
    )?;
    writeln!(&mut writer, "echo Linking resource DLL...")?;
    writeln!(
        &mut writer,
        r#"link /nologo /dll /noentry /machine:x86 "/out:%~dp0house.glr" "%~dp0resource.res""#
    )?;
    Ok(())
}

struct FinderIconBitmaps {
    large_8bit: Option<BitmapEight>,
    large_4bit: Option<BitmapFour>,
    large_1bit: Option<BitmapOne>,
    large_mask: BitmapOne,
    small_8bit: Option<BitmapEight>,
    small_4bit: Option<BitmapFour>,
    small_1bit: Option<BitmapOne>,
    small_mask: BitmapOne,
}

impl Default for FinderIconBitmaps {
    fn default() -> Self {
        Self {
            large_8bit: None,
            large_4bit: None,
            large_1bit: None,
            large_mask: BitmapOne::new(32, 32),
            small_8bit: None,
            small_4bit: None,
            small_1bit: None,
            small_mask: BitmapOne::new(16, 16),
        }
    }
}

fn collate_icons<W>(resfork: &ResourceFork, mut zipfile: &mut ZipWriter<W>) -> AnyResult<()>
where
    W: Seek + Write,
{
    let mut finder_icons: HashMap<i16, FinderIconBitmaps> = HashMap::new();
    for resource in resfork.iter() {
        let entry = finder_icons.entry(resource.id);
        match resource.restype.as_bstr() {
            b"icl8" => match large_8bit_icon::convert(&resource.data) {
                Ok(image) => entry.or_default().large_8bit = Some(image),
                Err(_) => report_invalid_resource(resource),
            },
            b"icl4" => match large_4bit_icon::convert(&resource.data) {
                Ok(image) => entry.or_default().large_4bit = Some(image),
                Err(_) => report_invalid_resource(resource),
            },
            b"ICN#" => match icon_list::convert(&resource.data) {
                Ok((image, mask)) => {
                    let entry = entry.or_default();
                    entry.large_1bit = Some(image);
                    entry.large_mask = mask;
                }
                Err(_) => report_invalid_resource(resource),
            },
            b"ics8" => match small_8bit_icon::convert(&resource.data) {
                Ok(image) => entry.or_default().small_8bit = Some(image),
                Err(_) => report_invalid_resource(resource),
            },
            b"ics4" => match small_4bit_icon::convert(&resource.data) {
                Ok(image) => entry.or_default().small_4bit = Some(image),
                Err(_) => report_invalid_resource(resource),
            },
            b"ics#" => match small_icon_list::convert(&resource.data) {
                Ok((image, mask)) => {
                    let entry = entry.or_default();
                    entry.small_1bit = Some(image);
                    entry.small_mask = mask;
                }
                Err(_) => report_invalid_resource(resource),
            },
            _ => {}
        }
    }
    for (icon_id, mut icon_entry) in finder_icons {
        let mut icon_file = IconFile::new();
        if let Some(image) = icon_entry.large_8bit.take() {
            icon_file.add_entry(image, icon_entry.large_mask.clone());
        }
        if let Some(image) = icon_entry.large_4bit.take() {
            icon_file.add_entry(image, icon_entry.large_mask.clone());
        }
        if let Some(image) = icon_entry.large_1bit.take() {
            icon_file.add_entry(image, icon_entry.large_mask.clone());
        }
        if let Some(image) = icon_entry.small_8bit.take() {
            icon_file.add_entry(image, icon_entry.small_mask.clone());
        }
        if let Some(image) = icon_entry.small_4bit.take() {
            icon_file.add_entry(image, icon_entry.small_mask.clone());
        }
        if let Some(image) = icon_entry.small_1bit.take() {
            icon_file.add_entry(image, icon_entry.small_mask.clone());
        }
        zipfile.start_file(format!("FinderIcon/{}.ico", icon_id), Default::default())?;
        icon_file.write_to(&mut zipfile)?;
    }
    Ok(())
}

fn convert_resfork(resfork: &ResourceFork, writer: impl Seek + Write) -> AnyResult<()> {
    let mut zipfile = ZipWriter::new(writer);
    zipfile.set_comment("");

    if resfork.iter().any(|res| res.name.is_some()) {
        zipfile.start_file("names.txt", Default::default())?;
        write_names_txt(resfork, &mut zipfile)?;
    }

    zipfile.start_file("resource.rc", Default::default())?;
    write_rc_script(resfork, &mut zipfile)?;

    zipfile.start_file("do_rle.cmd", Default::default())?;
    write_do_rle_script(&mut zipfile)?;

    zipfile.start_file("build.cmd", Default::default())?;
    write_build_script(resfork, &mut zipfile)?;

    for resource in resfork.iter() {
        let mut buffer = Vec::new();
        let (entry_name, output) = match convert_resource(resource, &mut buffer) {
            Ok(_) => (get_entry_name(resource), &buffer),
            Err(_) => (make_zip_entry_path(resource), &resource.data),
        };
        zipfile.start_file(entry_name, Default::default())?;
        zipfile.write_all(output)?;
    }

    for resource in resfork.iter_type(b"PAT#") {
        let patterns = match pattern_list::convert(&resource.data) {
            Ok(patterns) => patterns,
            Err(_) => {
                report_invalid_resource(resource);
                continue;
            }
        };
        for (idx, patt) in patterns.into_iter().enumerate() {
            let entry_name = format!("PatternList/{}/{}.bmp", resource.id, idx);
            zipfile.start_file(entry_name, Default::default())?;
            patt.write_bmp_file(&mut zipfile)?;
        }
    }

    collate_icons(resfork, &mut zipfile)?;

    Ok(())
}

fn parse_resfork<P: AsRef<Path>>(filename: P) -> io::Result<ResourceFork> {
    let file_reader = BufReader::new(File::open(filename)?);
    let rsrc_cursor = Cursor::new(extract_resource_bytes(file_reader)?);
    ResourceFork::read_from(rsrc_cursor)
}

fn do_derez_command<I>(args: I) -> Result<(), Box<dyn Error>>
where
    I: IntoIterator<Item = OsString>,
{
    let mut args = args.into_iter();
    let input_name = match args.next() {
        Some(s) => s,
        None => {
            invalid_cmdline();
            return Ok(());
        }
    };
    let output_name = args.next();

    let resfork = parse_resfork(input_name)?;
    match output_name {
        Some(filename) => {
            let output_file = BufWriter::new(File::create(filename)?);
            derez_resfork(&resfork, output_file)
        }
        None => derez_resfork(&resfork, io::stdout().lock()),
    }
}

fn do_dump_command<I>(args: I) -> Result<(), Box<dyn Error>>
where
    I: IntoIterator<Item = OsString>,
{
    let mut args = args.into_iter();
    let input_name = match args.next() {
        Some(s) => s,
        None => {
            invalid_cmdline();
            return Ok(());
        }
    };
    let output_name = match args.next() {
        Some(s) => s,
        None => {
            invalid_cmdline();
            return Ok(());
        }
    };

    let resfork = parse_resfork(input_name)?;
    let output_file = BufWriter::new(File::create(output_name)?);
    dump_resfork(&resfork, output_file)
}

fn do_convert_command<I>(args: I) -> Result<(), Box<dyn Error>>
where
    I: IntoIterator<Item = OsString>,
{
    let mut args = args.into_iter();
    let input_name = match args.next() {
        Some(s) => s,
        None => {
            invalid_cmdline();
            return Ok(());
        }
    };
    let output_name = match args.next() {
        Some(s) => s,
        None => {
            invalid_cmdline();
            return Ok(());
        }
    };

    let resfork = parse_resfork(input_name)?;
    let output_file = BufWriter::new(File::create(output_name)?);
    convert_resfork(&resfork, output_file)
}

fn run() -> Result<(), Box<dyn Error>> {
    let mut args = env::args_os().skip(1);
    let command = args.next();
    match command.as_ref().and_then(|s| s.to_str()) {
        Some("derez") => do_derez_command(args)?,
        Some("dump") => do_dump_command(args)?,
        Some("convert") => do_convert_command(args)?,
        Some("--help") => print_long_help(),
        None => print_short_help(),
        _ => invalid_cmdline(),
    }
    Ok(())
}

fn main() -> Result<(), Box<dyn Error>> {
    run()?;
    process::exit(EXIT_CODE.load(Ordering::SeqCst) as i32)
}

fn invalid_cmdline() {
    eprintln!(
        r#"error: invalid usage

Run "resfork-extract --help" for usage information."#
    );
}

fn print_short_help() {
    eprintln!(
        r#"resfork-extract
Extract information from a Macintosh resource fork

Run "resfork-extract --help" for usage information."#
    );
}

fn print_long_help() {
    eprintln!(
        r#"resfork-extract
Extract information from a Macintosh resource fork

USAGE:
    resfork-extract <subcommand>

FLAGS:
    --help          Print this help information

SUBCOMMANDS:
    derez <resource-fork> [<output-text>]

        Produce output similar to Apple's old DeRez resource decompiler.
        If <output-text> is omitted, then standard output is used.

    dump <resource-fork> <output-zip>

        Dump the raw bytes of each resource in the resource fork into
        its own entry in the output .zip file. Each resource is grouped
        under a directory named after its type (e.g., 'PICT' or 'DLOG'),
        and named after its ID number.

    convert <resource-fork> <output-zip>

        Like the 'dump' command, but converts any resources that it
        knows about into a more modern or human-readable format, if
        possible. If a conversion fails or the format is unknown, then
        the raw bytes are dumped. For example 'PICT' resources are
        converted into BMP image files, and 'snd ' resources are
        converted into AIFF audio files.

REMARKS:

    The resource fork input file can be in either an AppleDouble
    container file, a MacBinary container file, or the raw resource
    fork's bytes on its own. resfork-extract attempts to decode the
    input file as AppleDouble, then as MacBinary, then as raw bytes."#
    );
}
