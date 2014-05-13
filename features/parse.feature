Feature: Parse a blueprint

  Scenario: Parse a blueprint file into YAML

    When I run `snowcrash blueprint.apib` 
    Then the output should contain the content of file "ast.yaml"

  Scenario: Parse a blueprint input into YAML

    When I run `snowcrash` interactively
    When I pipe in the file "blueprint.apib"
    Then the output should contain the content of file "ast.yaml"

  Scenario: Parse a blueprint file into JSON

    When I run `snowcrash --format=json blueprint.apib` 
    Then the output should contain the content of file "ast.json"

  Scenario: Parse a blueprint input into JSON

    When I run `snowcrash --format=json` interactively
    When I pipe in the file "blueprint.apib"
    Then the output should contain the content of file "ast.json"
