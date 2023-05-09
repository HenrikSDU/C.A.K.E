//import 'dart:ffi';
import 'dart:io';

import 'package:flutter/material.dart';
import 'package:file_picker/file_picker.dart';

void main() {
  runApp(const Slicer());
}

class Slicer extends StatefulWidget {
  const Slicer({super.key});

  @override
  State<Slicer> createState() => _SlicerState();
}

class _SlicerState extends State<Slicer> {
  var selectedindex = 0;

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: "slicer",
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: HomePage(),
    );
  }
}

class HomePage extends StatefulWidget {
  @override
  State<HomePage> createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  var selectedindex = 0;

  @override
  Widget build(BuildContext context) {
    Widget page = SlicePage();
    switch (selectedindex) {
      case 0:
        page = SlicePage();
        break;
      case 1:
        page = UploadPage();
        break;
      default:
        throw UnimplementedError('unimplemented widget for $selectedindex');
    }

    return LayoutBuilder(builder: (context, constraints) {
      return Material(
        child: Row(
          children: [
            SafeArea(
              child: NavigationRail(
                extended: constraints.maxWidth >= 500,
                destinations: const [
                  NavigationRailDestination(
                      icon: Icon(Icons.cake_outlined), label: Text("Slicer")),
                  NavigationRailDestination(
                      icon: Icon(Icons.arrow_circle_up_outlined),
                      label: Text("Upload"))
                ],
                selectedIndex: selectedindex,
                onDestinationSelected: (value) {
                  setState(() {
                    selectedindex = value;
                  });
                },
              ),
            ),
            Expanded(
                child:
                  //Material( child:
                    page,),
            //))
          ],
        ),
      );
    });
  }
}

class SlicePage extends StatefulWidget {
  @override
  State<SlicePage> createState() => _SlicePageState();
}

class _SlicePageState extends State<SlicePage> {
  var selectedindex = 0;
  String? filepath;
  final pathError = const SnackBar(content: Text("Error selecting the file."));

  Future<String?> getFile() async {
    FilePickerResult? result = await FilePicker.platform.pickFiles();

    if (result != null) {
      String? path = result.files.single.path;
      if (path != null) {
        debugPrint(path);
        return path;
      } else {
        debugPrint("There was an error with the file selection!  path = null");
        return null;
      }
    } else {
      debugPrint("User cancelled the process");
      return null;
    }
  }

  Future<dynamic> runAutotrace(String path) async {
    String exe = 'lib\\autotrace.exe';
    List<String> args = [
      '-centerline',
      path,
      '-o',
      '${path.replaceFirst(".bmp", "")}.svg'
    ];
    Process process = await Process.start(exe, args);
    stdout.addStream(process.stdout);
    stderr.addStream(process.stderr);
    return await process.exitCode;
  }

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
      children: [
      Padding(
        padding: const EdgeInsets.fromLTRB(20.0, 75.0, 0.0, 20.0),
        child: Column(mainAxisAlignment: MainAxisAlignment.center, children: [
          const Text('Image slicing'),
          const SizedBox(height: 5),
          const Text('Accepted format(s): *.bmp'),
          const SizedBox(height: 5),
          ElevatedButton(
            onPressed: () async {
              String? path = await getFile();
              setState(() {
                filepath = path;
              });
              debugPrint("path is $filepath");
            },
            child: const Text("Select file"),
          ),
          const SizedBox(height: 5),
          ConstrainedBox(constraints: const BoxConstraints(maxWidth: 175),child: Align(alignment: Alignment.center,child: Text('Selected file: $filepath', maxLines: null, overflow: TextOverflow.fade,))),
          const SizedBox(height: 10),
          Expanded(child: Container(),),
          SizedBox(
            width: 175,
            height: 45,
            child: ElevatedButton(
              onPressed: () {
                if (filepath == 'Not found') {
                  debugPrint("SliceButton but Path is null, error");
                  showDialog(
                      context: context,
                      builder: (BuildContext context) {
                        return AlertDialog(
                          title: const Text("Error"),
                          content:
                              const Text("There was an error selecting the file"),
                          actions: [
                            TextButton(
                              onPressed: () {
                                Navigator.of(context).pop();
                              },
                              child: const Text("Ok"),
                            ),
                          ],
                        );
                      });
                } else {
                  setState(() {
                    runAutotrace(filepath ?? 'Not found');
                  });
                }
              },
              child: const Text("Slice!", textScaleFactor: 2,),
            ),
          ),
        ]),
      ),
      Padding(padding: const EdgeInsets.fromLTRB(20.0, 20.0, 20.0, 20.0), child: Image.file(File(filepath ?? 'lib\\exampleBMP.bmp'),
      width: 500, height: 500,),),
      ], 
    );
  }
}

class UploadPage extends StatefulWidget {
  @override
  State<UploadPage> createState() => _UploadPageState();
}

class _UploadPageState extends State<UploadPage> {
  @override
  Widget build(BuildContext context) {
    return const Material(
      child: Text('uninplemented'),
    );
  }
}
