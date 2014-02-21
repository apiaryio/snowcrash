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

  Scenario: Valide a valid blueprint file

    When I run `snowcrash --validate blueprint.apib`
    Then the output should contain:
    """
    OK
    """

  Scenario: Valide a valid blueprint input

    When I run `snowcrash --validate` interactively
    When I pipe in the file "blueprint.apib"
    Then the output should contain:
    """
    OK
    """

  Scenario: Valide an invalid blueprint file

    When I run `snowcrash --validate invalid_blueprint.apib`
    Then it should fail with:
    """
    error
    """

  Scenario: Valide an invalid blueprint input

    When I run `snowcrash --validate` interactively
    When I pipe in the file "invalid_blueprint.apib"
    Then it should fail with:
    """
    error
    """
